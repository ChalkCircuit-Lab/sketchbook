from django.db import models

# Create your models here.

PROGRAMMING_LANGUAGES = (
    ('C','C/C++'),
    ('UNITY','UNITY'),
    ('PYTHON', 'PYTHON'),
    ('JAVA','JAVA'),
    ('JAVASCRIPT','JAVASCRIPT'),
)

FRAMEWORKS = (
    ('Django','DJANGO'),
    ('OpenGL','OPENGL'),
    ('Selenium','SELENIUM'),
    ('P5','P5JS'),
    ('WinBGIM','WINBGIM'),
)

class Project(models.Model):
    title = models.CharField(max_length=100)
    description = models.CharField(max_length=250, blank=True)
    imageURL = models.CharField(max_length=1000, blank=True)

    githubURL = models.URLField(blank=True)
    youtubeURL = models.URLField(blank=True)
    itchURL = models.URLField(blank=True)
    playTestURL = models.URLField(blank=True)

    language = models.CharField(max_length=10, blank=True, choices=PROGRAMMING_LANGUAGES)
    framework = models.CharField(max_length=10, blank=True, choices=FRAMEWORKS)

    def __str__(self):
        return self.title