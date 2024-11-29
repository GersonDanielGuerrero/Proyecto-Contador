import datetime
from django.shortcuts import render
from rest_framework.views import APIView
from rest_framework.response import Response
from .models import Medidor, Medicion
from django.http import JsonResponse


class MedicionView(APIView):
    def get(self, request):
        mediciones = Medicion.objects.all()
        data = []
        data.append({
            'medidor': "medidor",
            'fecha': "2021-10-10",
            'potencia': 100,
            'consumo': 100
        })
        return JsonResponse(data, safe=False)
    def post(self, request):
        
        medidor = Medidor.objects.get(nombre=request.data['dispositivo'])
            
        potencia = request.data['potencia']
        consumo = request.data['consumo']
        fecha = datetime.datetime.now()
        
        if potencia < 0:
            potencia = 0
        if consumo < 0:
            consumo = 0        
        medidor.consumo += consumo
        medidor.save()
            
        print("Medidor: ", medidor, "Potencia: ", potencia, "Consumo: ", consumo, "Fecha: ", fecha)

        medicion = Medicion(medidor=medidor, fecha=fecha, potencia=potencia, consumo=consumo)
        medicion.save()
        
        
        return JsonResponse({'message': 'Medicion creada'}, status=201)
        
        