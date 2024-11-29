#Importaciones para urls
from django.urls import path
from .views import MedicionView

urlpatterns = [
    path('', MedicionView.as_view()),
]