
from django.urls import path
from . import views

app_name = 'portfolio'
urlpatterns = [
    path('', views.projectView, name='projectView'),
    path('<int:project_id>/', views.detail, name='detail'),
    path('<str:language>/', views.projectView, name='projectView'),
    path('<str:language>/<str:framework>/', views.projectView, name='projectView'),
]