
let GlobalData = null;


fetchData('/static/db/portfolio.json');



async function fetchData(path) {
    try {
        const res = await fetch(path);
        const data = await res.json();
        GlobalData = data;
        injectToHTML(data);
        return data;
    } catch (error) {
        console.error('Error fetching data:', error);
        throw error; // Rethrow the error if you want to handle it further
    }
}

function injectToHTML(data) {
    // console.log(data);

    let CategorySelector = document.getElementsByClassName('Showcase-module--CategorySelector');

    for(let i = 0; i < data['Fields'].length; i++) {
        button = document.createElement('button');
        button.setAttribute('class', 'CategorySelector--Button');
        button.innerHTML = data['Fields'][i]['Name'];
        button.setAttribute('onclick', 'changeCategoryTo(' + i + ')');
        // button.setAttribute('onmouseover', 'changeCategoryTo(' + i + ')');
        CategorySelector[0].appendChild(button);
    }

    changeCategoryTo(0);
    changeProjectTo(0, 0);
}


function changeCategoryTo(categoryIdx) {
    updateProjectButtons(categoryIdx);
    changeProjectTo(categoryIdx, 0);

    // Change the video
    let VideoPlayer = document.getElementsByClassName('Showcase-module--VideoPlayer');
    let iframe = VideoPlayer[0].getElementsByTagName('iframe')[0];
    iframe.setAttribute('src', GlobalData['Fields'][categoryIdx]['Projects'][0]['Youtube']);

    // Change the iframe's background color
    let iframeBackground = document.getElementsByClassName('Showcase-module--VideoPlayer');
    iframeBackground[0].style.backgroundColor = GlobalData['Fields'][categoryIdx]['Color'];

    // Change the button's background color
    let CategorySelector = document.getElementsByClassName('Showcase-module--CategorySelector');
    let buttons = CategorySelector[0].getElementsByTagName('button');
    for(let i = 0; i < buttons.length; i++) {
        buttons[i].style.backgroundColor = "";
    }
    buttons[categoryIdx].style.backgroundColor = GlobalData['Fields'][categoryIdx]['Color'];

    // console.log("Changed to category " + categoryIdx);

    // Change active category button's text to white
    for(let i = 0; i < buttons.length; i++) {
        buttons[i].style.color = "black";
    }
    buttons[categoryIdx].style.color = "white"
}

function updateProjectButtons(categoryIdx) {
    let ProjectSelector = document.getElementsByClassName('Showcase-module--ProjectSelector');

    // Remove all buttons
    let buttons = ProjectSelector[0].getElementsByTagName('button');
    while(buttons.length > 0) {
        buttons[0].parentNode.removeChild(buttons[0]);
    }

    // Create & Add new ones from the GlobalData var
    // console.log(GlobalData['Fields'][categoryIdx]['Projects'].length);

    for(let i = 0; i < GlobalData['Fields'][categoryIdx]['Projects'].length; i++) {
        button = document.createElement('button');
        button.setAttribute('class', 'ProjectSelector--Button');
        button.innerHTML = GlobalData['Fields'][categoryIdx]['Projects'][i]['Name'];
        // button.setAttribute('onclick', 'changeProjectTo(' + categoryIdx + ',' + i + ')');
        button.setAttribute('onmouseover', 'changeProjectTo(' + categoryIdx + ',' + i + ')');
        ProjectSelector[0].appendChild(button);
    }
}

function changeProjectTo(categoryIdx, projectIdx) {
    // Change the video
    let VideoPlayer = document.getElementsByClassName('Showcase-module--VideoPlayer');
    VideoPlayer[0].getElementsByTagName('iframe')[0].setAttribute('src', GlobalData['Fields'][categoryIdx]['Projects'][projectIdx]['Youtube']);

    // Change the description

    // Change the iframe's background color
    let iframeBackground = document.getElementsByClassName('Showcase-module--VideoPlayer');
    // iframeBackground[0].style.backgroundColor = GlobalData['Fields'][categoryIdx]['Projects'][projectIdx]['Color'];
    iframeBackground[0].style.backgroundColor = GlobalData['Fields'][categoryIdx]['Color'];

    // Change the button's background color
    let ProjectSelector = document.getElementsByClassName('Showcase-module--ProjectSelector');
    let buttons = ProjectSelector[0].getElementsByTagName('button');
    for(let i = 0; i < buttons.length; i++) {
        buttons[i].style.backgroundColor = "";
    }
    // buttons[projectIdx].style.backgroundColor = GlobalData['Fields'][categoryIdx]['Projects'][projectIdx]['Color'];
    buttons[projectIdx].style.backgroundColor = GlobalData['Fields'][categoryIdx]['Color'];

    // Change active project button's text to white
    for(let i = 0; i < buttons.length; i++) {
        buttons[i].style.color = "black";
    }
    buttons[projectIdx].style.color = "white";
}