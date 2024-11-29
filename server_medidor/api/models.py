from django.db import models

class Medidor(models.Model):
    nombre = models.CharField(max_length=50)
    consumo = models.BigIntegerField()
    
class Medicion(models.Model):
    medidor = models.ForeignKey(Medidor, on_delete=models.CASCADE)
    fecha = models.DateField()
    potencia = models.BigIntegerField()
    consumo = models.BigIntegerField()