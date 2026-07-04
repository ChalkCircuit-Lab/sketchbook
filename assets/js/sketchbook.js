const chapterTitles = {
  "html-css": "HTML & CSS",
  bootstrap: "Bootstrap",
  javascript: "JavaScript",
  react: "React",
  "ui-patterns": "UI Patterns",
  java: "Java",
  c: "C",
  databases: "Databases",
  algorithms: "Algorithms",
  "distributed-systems": "Distributed Systems",
  "api-patterns": "API Patterns"
};

const areaTitles = {
  frontend: "Frontend",
  backend: "Backend",
  fullstack: "Fullstack",
  archive: "Archive"
};

const chapterOrder = [
  "html-css",
  "bootstrap",
  "javascript",
  "react",
  "ui-patterns",
  "java",
  "c",
  "databases",
  "algorithms",
  "distributed-systems",
  "api-patterns"
];

const state = {
  projects: [],
  filter: "all"
};

const projectRoot = document.querySelector("#projects");
const statusNode = document.querySelector("#status");
const filterButtons = document.querySelectorAll("[data-filter]");

function text(value, fallback = "Not recorded") {
  if (Array.isArray(value)) {
    return value.length ? value.join(", ") : fallback;
  }

  return value || fallback;
}

function sortProjects(projects) {
  return [...projects].sort((a, b) => {
    const areaSort = (a.area || "").localeCompare(b.area || "");
    if (areaSort) return areaSort;

    const chapterSort =
      chapterOrder.indexOf(a.chapter) - chapterOrder.indexOf(b.chapter);
    if (chapterSort) return chapterSort;

    return (a.title || a.id || "").localeCompare(b.title || b.id || "");
  });
}

function groupBy(projects, key) {
  return projects.reduce((groups, project) => {
    const value = project[key] || "unlisted";
    groups[value] = groups[value] || [];
    groups[value].push(project);
    return groups;
  }, {});
}

function projectCard(project) {
  const article = document.createElement("article");
  article.className = "project-card";

  const title = document.createElement("h4");
  title.textContent = project.title || project.id || "Untitled project";
  article.append(title);

  const meta = document.createElement("div");
  meta.className = "meta";
  meta.textContent = `${text(project.status)} / ${text(project.teachingLevel)} / source: ${text(project.sourceRepo)}`;
  article.append(meta);

  const description = document.createElement("p");
  description.textContent = text(project.description, "No description yet.");
  article.append(description);

  const concepts = document.createElement("div");
  concepts.className = "meta";
  concepts.textContent = `Concepts: ${text(project.concepts)}`;
  article.append(concepts);

  const links = document.createElement("div");
  links.className = "links";

  if (project.originalPath) {
    const original = document.createElement("a");
    original.href = project.originalPath;
    original.textContent = "archived original";
    links.append(original);
  }

  if (project.cleanPath) {
    const clean = document.createElement("a");
    clean.href = project.cleanPath;
    clean.textContent = "clean target";
    links.append(clean);
  }

  article.append(links);

  const tags = document.createElement("div");
  tags.className = "tags";
  (project.tags || []).forEach((tag) => {
    const span = document.createElement("span");
    span.className = "tag";
    span.textContent = tag;
    tags.append(span);
  });
  article.append(tags);

  return article;
}

function render() {
  const filtered =
    state.filter === "all"
      ? state.projects
      : state.projects.filter((project) => project.area === state.filter);

  projectRoot.replaceChildren();
  statusNode.textContent = `${filtered.length} of ${state.projects.length} projects shown`;

  if (!filtered.length) {
    const empty = document.createElement("p");
    empty.className = "empty";
    empty.textContent = "No projects match this filter yet.";
    projectRoot.append(empty);
    return;
  }

  const byArea = groupBy(sortProjects(filtered), "area");

  Object.keys(byArea)
    .sort((a, b) => Object.keys(areaTitles).indexOf(a) - Object.keys(areaTitles).indexOf(b))
    .forEach((area) => {
      const section = document.createElement("section");
      section.className = "area";

      const heading = document.createElement("h2");
      heading.textContent = areaTitles[area] || area;
      section.append(heading);

      const byChapter = groupBy(byArea[area], "chapter");
      Object.keys(byChapter)
        .sort((a, b) => chapterOrder.indexOf(a) - chapterOrder.indexOf(b))
        .forEach((chapter) => {
          const chapterSection = document.createElement("section");
          chapterSection.className = "chapter";

          const chapterHeading = document.createElement("h3");
          chapterHeading.textContent = chapterTitles[chapter] || chapter;
          chapterSection.append(chapterHeading);

          const grid = document.createElement("div");
          grid.className = "project-grid";
          byChapter[chapter].forEach((project) => grid.append(projectCard(project)));
          chapterSection.append(grid);
          section.append(chapterSection);
        });

      projectRoot.append(section);
    });
}

filterButtons.forEach((button) => {
  button.addEventListener("click", () => {
    state.filter = button.dataset.filter;
    filterButtons.forEach((item) =>
      item.setAttribute("aria-pressed", String(item === button))
    );
    render();
  });
});

fetch("data/projects.json")
  .then((response) => {
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    return response.json();
  })
  .then((projects) => {
    state.projects = Array.isArray(projects) ? projects : [];
    render();
  })
  .catch((error) => {
    projectRoot.innerHTML = "";
    const message = document.createElement("p");
    message.className = "error";
    message.textContent =
      "Could not load data/projects.json. Open this page through a local web server and check that the metadata file is valid JSON.";
    projectRoot.append(message);
    statusNode.textContent = error.message;
  });
