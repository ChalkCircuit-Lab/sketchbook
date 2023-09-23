from django.shortcuts import render
from django.http import HttpResponse
from django.shortcuts import render, get_object_or_404


from .models import Project, PROGRAMMING_LANGUAGES, FRAMEWORKS

def projectView(request, language=None, framework=None):
    programming_languages = dict(PROGRAMMING_LANGUAGES)
    active_language = language

    frameworks = dict(FRAMEWORKS)
    active_framework = framework

    projects = Project.objects.all()

    if language in programming_languages:  # Check if the language is a valid key
        projects = projects.filter(language=language)

    if framework in frameworks:  # Check if the framework is a valid key
        projects = projects.filter(framework=framework)

    context = {
        'projects': projects,
        'programming_languages': programming_languages,
        'active_language': active_language,

        # 'frameworks': frameworks,
        # 'active_framework': active_framework,
    }
        
    return render(request, 'portfolio/projects.html', context)

def detail(request, project_id):
    project = get_object_or_404(Project, pk=project_id)
    print(project)
    return HttpResponse("<h2>Details for Project id: " + str(project_id) + "</h2>")