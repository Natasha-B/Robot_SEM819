# -*- coding: utf-8 -*-
"""
Created on Thu May  2 08:44:28 2019

@author: David
"""

import json
import numpy as np

def conversion_unite(unite,valeur): #convertit une valeur par défaut en cm dans une autre unite

    if unite == "cm":
        return valeur
    elif unite == "dm":
        return valeur/10
    elif unite == "m":
        return valeur/100
    elif unite == "mm":
        return valeur*10

def conversion_angle(unite,valeur): #convertit une valeur en radian en une valeur entiere en degré si l'unite est "degre"
    if unite == "radian":
        return int(round(valeur))
    else:
        return int(round(valeur * 180 / np.pi))


#ouverture et stockage des données des fichiers json
f_envir = 'Description_Environnement_SansAccent_V10-05-17.js'
f_base_roul = 'Evolution_Base_Roulante_SansAccent_V10-05-17.js'

with open(f_envir) as json_data_envir:
    data_dict_envir = json.load(json_data_envir) 
with open(f_base_roul) as json_data_base_roul:
    data_dict_base_roul = json.load(json_data_base_roul)
    



unite_envir = data_dict_envir.get("unite")
unite_base_roul = data_dict_base_roul.get("unite")
unite_ang = data_dict_base_roul.get("unite_angulaire")

#coordonnées de la cible
cible_x = data_dict_envir.get("cible").get("centre").get("x")
cible_y = data_dict_envir.get("cible").get("centre").get("y")
cible_z = data_dict_envir.get("cible").get("hauteur")

#coordonnées de la position de tir
pos_tir_x = data_dict_base_roul.get("position-tir_cible").get("coordonnees").get("x")
pos_tir_y = data_dict_base_roul.get("position-tir_cible").get("coordonnees").get("y")
pos_tir_z = conversion_unite(unite_base_roul,31)   #hauteur du pointeur lumineux = 31 cm quand angle = 90°


if unite_envir == unite_base_roul:
    distance_x = cible_x - pos_tir_x
    distance_y = cible_y - pos_tir_y
    distance_z = cible_z - pos_tir_z
#if unite_envir == unite_base_roul:
#    distance_x = 0
#    distance_y = 75
#    distance_z = 57
else:
    #calcul des distances entre position tir et cible selon les différentes unité
    if unite_base_roul == "cm":
        distance_x = cible_x - conversion_unite(unite_envir,pos_tir_x)
        distance_y = cible_y - conversion_unite(unite_envir,pos_tir_y)
        distance_z = cible_z - conversion_unite(unite_envir,pos_tir_z)
    elif unite_base_roul == "dm":
        distance_x = cible_x - conversion_unite(unite_envir,pos_tir_x*10)
        distance_y = cible_y - conversion_unite(unite_envir,pos_tir_y*10)
        distance_z = cible_z - conversion_unite(unite_envir,pos_tir_z*10)
    elif unite_base_roul == "m":
        distance_x = cible_x - conversion_unite(unite_envir,pos_tir_x*100)
        distance_y = cible_y - conversion_unite(unite_envir,pos_tir_y*100)
        distance_z = cible_z - conversion_unite(unite_envir,pos_tir_z*100)
    elif unite_base_roul == "mm":
        distance_x = cible_x - conversion_unite(unite_envir,pos_tir_x/10)
        distance_y = cible_y - conversion_unite(unite_envir,pos_tir_y/10)
        distance_z = cible_z - conversion_unite(unite_envir,pos_tir_z/10)

distance_plan_horizontal = np.sqrt(distance_x**2 + distance_y**2)
angle_plan_horizontal = np.arccos(distance_y/(distance_plan_horizontal)) #angle en radians
angle_plan_horizontal = conversion_angle(unite_ang,angle_plan_horizontal)
    
    
distance_plan_vertical = np.sqrt(distance_plan_horizontal**2 + distance_z**2)
angle_plan_vertical = np.arccos(distance_z/(distance_plan_vertical)) #angle en radians
angle_plan_vertical  = conversion_angle(unite_ang,angle_plan_vertical )

commande_h = "CS H A:"+str(angle_plan_horizontal)
commande_v = "CS V A:"+str(- angle_plan_vertical)
commande_tir = "L 100 10 0 1"  # tir avec intensité lumineuse = 100% et durée d'allumage 1s 1 fois
commande_fin_tir = "LS"


#    Creation du fichier texte avec les commandes
f = open("parcours_optimal.txt","a")
f.write("%s\r\n" %(commande_h))
f.write("%s\r\n" %(commande_v))
f.write("%s\r\n" %(commande_tir))
f.write("%s\r\n" %(commande_fin_tir))

    
f.close()