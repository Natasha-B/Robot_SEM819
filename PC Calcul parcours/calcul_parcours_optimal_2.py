# -*- coding: utf-8 -*-
"""
Éditeur de Spyder

Ceci est un script temporaire.
"""
import numpy as np
import json
import matplotlib.pyplot as plt
from heapq import *

#ouverture et stockage des données des fichiers json
f_envir = 'Description_Environnement_SansAccent_V10-05-17.js'
f_base_roul = 'Evolution_Base_Roulante_SansAccent_V10-05-17.js'

with open(f_envir) as json_data_envir:
    data_dict_envir = json.load(json_data_envir) 
with open(f_base_roul) as json_data_base_roul:
    data_dict_base_roul = json.load(json_data_base_roul)



""" Déroulement scénario
- se mettre à la position de départ
- calculer nombre n etapes
- i = 1
- while i<n
    - calculer la position des obstacles à identifier avant i
    - calcul parcours optimal jusqu'à étape i apres avoir identifié les balises-obstacles
- aller à la position tir-cible
- donner coordonnées pour tir ?
- aller à l'arrivée
"""


#création de la matrice représentant le terrain
#on met des 0 sur les cases dispo et des 1 pour les cases non dispo
taille_robot = 45  #robot = 45cm²
terrain_data_dict = data_dict_envir.get("terrain_evolution")  #dictionaire contenant les infos du terrain
coord_terrain = []
coord_terrain_x = []
coord_terrain_y = []
size_terrain_dict = len(terrain_data_dict)
for i in range(size_terrain_dict):
    coord_terrain.append(terrain_data_dict[i].get("point"))
for j in range(size_terrain_dict):
    coord_terrain_x.append(int(coord_terrain[j].get("x"))) #coordonnées en x des sommets du terrain
    coord_terrain_y.append(int(coord_terrain[j].get("y"))) #coordonnées en y des sommets du terrain
    """ Avec une autre echelle"""
    #coord_terrain_x.append(int(coord_terrain[j].get("x")/taille_robot)) #coordonnées en x des sommets du terrain
    #coord_terrain_y.append(int(coord_terrain[j].get("y")/taille_robot)) #coordonnées en y des sommets du terrain

        
""" Création de listes liées pour coord x et y"""        
dict_x_sorted0 = {} #on crée un dictionnaire pour trier les valeurs de terrain_cord_x
list_y_sorted = [(0,0) for i in range(size_terrain_dict)]#on crée un ditionnaire pour trier les valeurs de terrain_coord_y selon l'ordre précédent
for i in range(size_terrain_dict):
    dict_x_sorted0[i] = coord_terrain_x[i]   
    
def sorted_Value(elem):  #fonction utilisée pour le tri de dictionnaire
    return elem[1]
    
list_x_sorted = sorted(dict_x_sorted0.items(),key=sorted_Value)
for i in range(size_terrain_dict):
    list_y_sorted[i] = (list_x_sorted[i][0],coord_terrain_y[list_x_sorted[i][0]])
    
"""Création de la modélisation du terrain en grille"""    

min_x = min(coord_terrain_x)
max_x = max(coord_terrain_x)
min_y = min(coord_terrain_y)
max_y = max(coord_terrain_y)

terrain_grille = []
for i in range(max_y-min_y+1):  # de 0 à 501 (vertical)
        terrain_grille.append([1]*(max_x-min_x+1))  #on remplit la grille de 1
        
for i in range(size_terrain_dict):  # de 0 à 501 (vertical)
    if i%2==0: #on prend un indice sur deux car 
        a = min(list_y_sorted[i][1]+abs(min_y),list_y_sorted[i+1][1]+abs(min_y))
        if a > 0:
            a +=1
        b = max(list_y_sorted[i][1]+abs(min_y),list_y_sorted[i+1][1]+abs(min_y))
        if b > 0:
            b +=1

        for j in range(a,b):
            if min_x <0:
                terrain_grille[j][list_x_sorted[i][1]+abs(min_x)]=0
            else:
                terrain_grille[j][list_x_sorted[i][1]-(min_x)]=0

            
for i in range(len(terrain_grille)):
    kx = 0
    #permet de trouver l'abscisse x où débute la zone où le robot peut circuler
    while terrain_grille[i][kx] >0:
        kx +=1
    kx += 1
    #on remplit la zone où le robot peut circuler de 0, jusqu'à qu'on trouve la limite de la zone ie jusqu'à qu'on tombe sur un 1
    while (terrain_grille[i][kx] >0) & (kx<max_x+abs(min_x)):   
        #kx += 1
        terrain_grille[i][kx] =0
        kx += 1

terrain_numpy = np.array(terrain_grille)   

#simulation des obstacles sur le terrain
def object_on_map(obj,terrain,value=1,min_x_terrain=0,min_y_terrain=0,marge=0):
    #simule un objet obj sur le terrain grâce à la liste des coordonnées de ses sommets
    #puis remplit son interieur de la valeur value
    if marge<0:
        print("Erreur ! Choisissez une marge nulle ou positive")
        return
    if taille_robot<0:
        print("Erreur ! Le robot ne peut avoir une taille négative ! ")
        return
    coord_obj = []
    coord_obj_x = []
    coord_obj_y = []
    list_sommet_ob_i = obstacle_i_dict.get('coordonnees')

    if obstacle_i_dict.get('type') == "cercle": #si l'obstacle est de forme circulaire
        #coordonées du centre du cercle 
        centre_coord_x = ((list_sommet_ob_i[0].get('centre')).get("x"))
        centre_coord_y = ((list_sommet_ob_i[0].get('centre')).get("y"))
        r = (list_sommet_ob_i[1].get('rayon'))     #rayon du cercle 
        """Avec autre echelle """
        #centre_coord_x = ((list_sommet_ob_i[0].get('centre')).get("x"))/taille_robot
        #centre_coord_y = ((list_sommet_ob_i[0].get('centre')).get("y"))/taille_robot
        #r = (list_sommet_ob_i[1].get('rayon'))/ taille_robot  #rayon du cercle 
        
        min_x = int(centre_coord_x -r - min_x_terrain)
        max_x = int(centre_coord_x + r - min_x_terrain) 
        min_y = int(centre_coord_y - r - min_y_terrain) 
        max_y = int(centre_coord_y + r - min_y_terrain) 
    else: # si l'obstacle n'est pas circulaire
    
        size_obj_dict = len(list_sommet_ob_i)
        for i in range(size_obj_dict):
            coord_obj.append(list_sommet_ob_i[i].get("point"))
        for j in range(size_obj_dict):
            coord_obj_x.append((coord_obj[j].get("x"))) #coordonnées en x des sommets du terrain
            coord_obj_y.append((coord_obj[j].get("y"))) #coordonnées en y des sommets du terrain
        
            """Avec autre echelle """
            #coord_obj_x.append((coord_obj[j].get("x"))/taille_robot) #coordonnées en x des sommets du terrain
            #coord_obj_y.append((coord_obj[j].get("y"))/taille_robot) #coordonnées en y des sommets du terrain
        """ Création de listes liées pour coord x et y"""        
        dict_x_sorted0 = {} #on crée un dictionnaire pour trier les valeurs de terrain_cord_x
        list_y_sorted = [(0,0) for i in range(size_obj_dict)]#on crée un ditionnaire pour trier les valeurs de terrain_coord_y selon l'ordre précédent
        for i in range(size_obj_dict):
            dict_x_sorted0[i] = coord_obj_x[i]   
            
        def sorted_Value(elem):  #fonction utilisée pour le tri de dictionnaire
            return elem[1]
            
        list_x_sorted = sorted(dict_x_sorted0.items(),key=sorted_Value)
        for i in range(size_obj_dict):
            list_y_sorted[i] = (list_x_sorted[i][0],coord_obj_y[list_x_sorted[i][0]])
            
        """Création de la modélisation du terrain en grille"""    
        """condition type"""
        min_x = int(min(coord_obj_x) - min_x_terrain)
        max_x = int(max(coord_obj_x) - min_x_terrain)
        min_y = int(min(coord_obj_y) - min_y_terrain)
        max_y = int(max(coord_obj_y) - min_y_terrain) 
    """ on modelise toutes les surfaces des obstacles par un rectangle"""
   
    for i in range(min_y, max_y+1):  # de 0 à 501 (vertical)
        for j in range(min_x, max_x +1): 
            terrain[i][j] = value
    return terrain
            
    
terrain_numpy2 = terrain_numpy.copy()
obstacle_data_dict = data_dict_envir.get("obstacles")
n_obstacles = len(obstacle_data_dict)

marge = 0
value = 1 # valeur sur la map représentant les obstacles
for i in range (n_obstacles):
    obstacle_i_dict = obstacle_data_dict[i]
    object_on_map(obstacle_i_dict,terrain_numpy2,value,min_x,min_y,marge)
    
"""  debut balises"""
balise_data_dict = data_dict_envir.get("balises")
if balise_data_dict != None:
    n_balises = len(balise_data_dict)
else:
    print("Pas de balises")
    n_balises = 0
""" fin balises """

     #object_on_map(obstacle_i_dict,terrain_numpy2,value=1,min_x,min_y,marge=0,taille_robot)
#    plt.figure(i+3)
#    plt.imshow(terrain_numpy2)
      

plt.figure(1)
plt.imshow(terrain_numpy2)

###  TEST A STAR ###

def heuristic(a, b):
    return (b[0] - a[0]) ** 2 + (b[1] - a[1]) ** 2

def astar(array, start, goal,taille_robot=1):

    #neighbors = [(0,1),(0,-1),(1,0),(-1,0),(1,1),(1,-1),(-1,1),(-1,-1)]         
    #neighbors = [(0,1),(0,-1),(1,0),(-1,0)]
    neighbors = []
    for i in range(-taille_robot, taille_robot):
        neighbors.append((taille_robot,i))
        neighbors.append((-taille_robot,i))
        neighbors.append((i,taille_robot))
        neighbors.append((i,-taille_robot))
    close_set = set()
    came_from = {}
    gscore = {start:0}
    fscore = {start:heuristic(start, goal)}
    oheap = []

    heappush(oheap, (fscore[start], start))
    
    while oheap:

        current = heappop(oheap)[1]

        if current == goal:
            data = []
            while current in came_from:
                data.append(current)
                current = came_from[current]
            return data

        close_set.add(current)
        for i, j in neighbors:
            neighbor = current[0] + i, current[1] + j            
            tentative_g_score = gscore[current] + heuristic(current, neighbor)
            if 0 <= neighbor[0] < array.shape[0]:
                if 0 <= neighbor[1] < array.shape[1]:                
                    if array[neighbor[0]][neighbor[1]] == 1:
                        
                        continue
                else:
                    # array bound y walls
                    continue
            else:
                # array bound x walls
                continue
                
            if neighbor in close_set and tentative_g_score >= gscore.get(neighbor, 0):
                continue
                
            if  tentative_g_score < gscore.get(neighbor, 0) or neighbor not in [i[1]for i in oheap]:
                came_from[neighbor] = current
                gscore[neighbor] = tentative_g_score
                fscore[neighbor] = tentative_g_score + heuristic(neighbor, goal)
                heappush(oheap, (fscore[neighbor], neighbor))
    return False


#coordonnées des points importants (départ, arrivée, tir et étapes)
depart = data_dict_base_roul.get("depart")
depart_x0 = (depart.get("coordonnees")).get("x")
depart_y0 = (depart.get("coordonnees")).get("y")

arrivee = data_dict_base_roul.get("arrivee")
arrivee_x0 = (arrivee.get("coordonnees")).get("x")
arrivee_y0 = (arrivee.get("coordonnees")).get("y")

tir = (data_dict_base_roul.get("position-tir_cible")).get("coordonnees")
tir_x0 = tir.get("x")
tir_y0 = tir.get("y")

def ajustement_coord(min_coord_terrain,coord_0):
    # fonction permettant d'ajuster les coordonées selon les min du terrain
    if min_coord_terrain <0:
        return coord_0 + abs(min_coord_terrain)
    else:
       return coord_0 - min_coord_terrain

depart_x = ajustement_coord(min_x,depart_x0)    
depart_y = ajustement_coord(min_y,depart_y0) 
arrivee_x = ajustement_coord(min_x,arrivee_x0) 
arrivee_y = ajustement_coord(min_y,arrivee_y0) 
tir_x = ajustement_coord(min_x,tir_x0) 
tir_y = ajustement_coord(min_y,tir_y0) 

nb_etapes = len(data_dict_base_roul.get("etapes")) 
list_x_etapes = []
list_y_etapes = []

list_x_balises = []
list_y_balises = []

list_x_move = []
list_y_move = []

if n_balises >0:
    for i in range(n_balises):  
        balise_data_dict 
        balise_i_x =(balise_data_dict [i].get("coordonnees")).get("x")
        balise_i_y =(balise_data_dict [i].get("coordonnees")).get("y")
        list_x_balises.append(ajustement_coord(min_x, balise_i_x))
        list_y_balises.append(ajustement_coord(min_y, balise_i_y))
    
for i in range(nb_etapes):  
    etape_i_x =((data_dict_base_roul.get("etapes"))[i].get("coordonnees")).get("x")
    etape_i_y =((data_dict_base_roul.get("etapes"))[i].get("coordonnees")).get("y")
    list_x_etapes.append(ajustement_coord(min_x, etape_i_x))
    list_y_etapes.append(ajustement_coord(min_y, etape_i_y))
    
#calcul du parcours pour chaque trajet:    
taille_r = 1
for i in range(nb_etapes + 2):
    if i == 0 : #entre départ et 1ere étape
        way = astar(terrain_numpy2, (list_y_etapes[i],list_x_etapes[i]), (depart_y,depart_x),taille_r)
        for i in range(len(way)):
            list_x_move.append(way[i][1])
            list_y_move.append(way[i][0])
        plt.figure(1)
        plt.plot(list_x_move,list_y_move)


    elif i == nb_etapes :
        #on rajoute ces deux lignes pour créer une répétition pour trouver quand on arrive au stand de tir
        list_x_move.append(list_x_etapes[i-1])
        list_y_move.append(list_y_etapes[i-1])
        list_x_move.append(list_x_etapes[i-1])
        list_y_move.append(list_y_etapes[i-1])
        way = astar(terrain_numpy2, (tir_y,tir_x), (list_y_etapes[i-1]+1,list_x_etapes[i-1]+1),taille_r)
        for i in range(len(way)):
            list_x_move.append(way[i][1])
            list_y_move.append(way[i][0])
        plt.figure(1)
        plt.plot(list_x_move,list_y_move)
    elif i == nb_etapes + 1:
        list_x_move.append(tir_x)
        list_x_move.append(tir_x)
        list_y_move.append(tir_y)
        list_y_move.append(tir_y)
        way = astar(terrain_numpy2, (arrivee_y,arrivee_x), (tir_y+1,tir_x+1),taille_r)
        for i in range(len(way)):
            list_x_move.append(way[i][1])
            list_y_move.append(way[i][0])
            plt.figure(1)
            plt.plot(list_x_move,list_y_move)
                
    else:
        list_x_move.append(list_x_etapes[i-1])
        list_x_move.append(list_x_etapes[i-1])
        list_y_move.append(list_y_etapes[i-1])
        list_y_move.append(list_y_etapes[i-1])
        way = astar(terrain_numpy2, (list_y_etapes[i],list_x_etapes[i]), (list_y_etapes[i-1]+1,list_x_etapes[i-1]+1),taille_r)
        for i in range(len(way)):
            list_x_move.append(way[i][1])
            list_y_move.append(way[i][0])
            plt.figure(1)
            
            
            """essayer avec autre echelle : une case = 6.25 unite² et grossir les obstacles"""
            """modifier calcul pointeur lumineux + calcul parcours afin de réécrire 
            fichier commande et rajouter commadne tir au milieu du parcours"""
            
            
            plt.plot(list_x_move,list_y_move)
            
# Création des commandes de déplacement en fonction de la liste des déplacements précédemment créés
list_command = ["droite"]
i =0
i_tir = 0
while i < len(list_x_move) -2:
    if i_tir == 0:
        if (list_x_move[i] ==tir_x) & (list_y_move[i] ==tir_y):
            list_command.append("tir")
            i_tir +=1
    for k in range(nb_etapes):
        if (list_x_move[i] ==list_x_etapes[k]) & (list_y_move[i] ==list_y_etapes[k]):
            list_command.append("etape")    
    if list_x_move[i] == list_x_move[i+1]:
        if list_y_move[i] < list_y_move[i+1]:
            list_command.append("haut")
            i +=1
        if list_y_move[i] > list_y_move[i+1]:
            list_command.append("bas")
            i +=1
        if list_y_move[i] == list_y_move[i+1]:
            list_command.append("S")
            i +=1
    if list_y_move[i] == list_y_move[i+1]:
        if list_x_move[i] < list_x_move[i+1]:
            list_command.append("droite")
            i +=1
        if list_x_move[i] > list_x_move[i+1]:
            list_command.append("gauche")
            i +=1
    if list_x_move[i] - list_x_move[i+1] == -1:
        if list_y_move[i] - list_y_move[i+1] == -1:
            list_command.append("haut droite")
            i +=1
        if list_y_move[i] - list_y_move[i+1] == 1:
            list_command.append("bas droite")
            i +=1
    if list_x_move[i] - list_x_move[i+1] == 1:
        if list_y_move[i] - list_y_move[i+1] == -1:
            list_command.append("haut gauche")
            i +=1
        if list_y_move[i] - list_y_move[i+1] == 1:
            list_command.append("bas gauche")
            i +=1

list_command.append("Fin")  #permet d'avoir la dernière commande de déplacement                   
i = 0                    
compt_avance = 1
list_command2 = []
while i < len(list_command)-1:
    command = list_command[i]
    command_next = list_command[i+1]
    if command == command_next:
        compt_avance +=1 
        i += 1
    else:
        compt_avance +=1
        if data_dict_envir.get("unite") == "mm":
            avance = compt_avance /100
        elif data_dict_envir.get("unite") == "cm":
            avance = compt_avance /10
        elif data_dict_envir.get("unite") == "dm":
            avance = compt_avance
        elif data_dict_envir.get("unite") == "m":
            avance = compt_avance * 10
        compt_avance = 0
        if command == "haut":
            if command_next == "bas":
                order = "G X:0 Y:"+ str(avance)+" A:180"
                list_command2.append(order)
                i += 1
            elif command_next == "gauche":
                order = "G X:0 Y:"+ str(avance)+" A:90"
                list_command2.append(order)
                i += 1
            elif command_next == "droite":
                order = "G X:0 Y:"+ str(avance)+" A:-90"
                list_command2.append(order)
                i += 1
            elif command_next == "haut gauche":
                order = "G X:0 Y:"+ str(avance)+" A:45"
                list_command2.append(order) ### que 90/-90 ?
                i += 1
            elif command_next == "haut droite":
                order = "G X:0 Y:"+ str(avance)+" A:-45"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-135"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas droite":
                order = "G X:0 Y:"+ str(avance)+" A:135"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "tir":
                list_command2.append("tir")
                i += 1
            elif command_next == "etape":
                list_command2.append("RC D")
                list_command2.append("RC D")
                i += 1
            else:
                order = "G X:0 Y:"+ str(avance)+" A:0"
                list_command2.append(order)
                list_command2.append("S")
                i+=1
        elif command == "bas":############################################
            if command_next == "haut":
                order = "G X:0 Y:"+ str(avance)+" A:180"
                list_command2.append(order)
                i += 1
            elif command_next == "gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-90"
                list_command2.append(order)
                i += 1
            elif command_next == "droite":
                order = "G X:0 Y:"+ str(avance)+" A:90"
                i += 1
                list_command2.append(order)
            elif command_next == "haut gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-135"
                list_command2.append(order) ### que 90/-90 ?
                i += 1
            elif command_next == "haut droite":
                order = "G X:0 Y:"+ str(avance)+" A:135"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-45"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas droite":
                order = "G X:0 Y:"+ str(avance)+" A:45"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "tir":
                list_command2.append("tir")
                i += 1
            elif command_next == "etape":
                list_command2.append("RC D")
                list_command2.append("RC D")
                i += 1
            else:
                order = "G X:0 Y:"+ str(avance)+" A:0"
                list_command2.append(order)
                list_command2.append("S")
                i+=1
        elif command == "gauche": #########################################
            if command_next == "haut":
                order = "G X:0 Y:"+ str(avance)+" A:-90"
                list_command2.append(order)
                i += 1
            elif command_next == "bas":
                order = "G X:0 Y:"+ str(avance)+" A:90"
                list_command2.append(order)
                i += 1
            elif command_next == "droite":
                order = "G X:0 Y:"+ str(avance)+" A:180"
                list_command2.append(order)
                i += 1
            elif command_next == "haut gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-45"
                list_command2.append(order) ### que 90/-90 ?
                i += 1
            elif command_next == "haut droite":
                order = "G X:0 Y:"+ str(avance)+" A:-135"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas gauche":
                order = "G X:0 Y:"+ str(avance)+" A:45"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas droite":
                order = "G X:0 Y:"+ str(avance)+" A:135"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "tir":
                list_command2.append("tir")
                i += 1
            elif command_next == "etape":
                list_command2.append("RC D")
                list_command2.append("RC D")
                i += 1
            else:
                order = "G X:0 Y:"+ str(avance)+" A:0"
                list_command2.append(order)
                list_command2.append("S")
                i+=1
        elif command == "droite": #########################################
            if command_next == "haut":
                order = "G X:0 Y:"+ str(avance)+" A:90"
                list_command2.append(order)
                i += 1
            elif command_next == "bas":
                order = "G X:0 Y:"+ str(avance)+" A:-90"
                list_command2.append(order)
                i += 1
            elif command_next == "gauche":
                order = "G X:0 Y:"+ str(avance)+" A:180"
                list_command2.append(order)
                i += 1
            elif command_next == "haut gauche":
                order = "G X:0 Y:"+ str(avance)+" A:135"
                list_command2.append(order) ### que 90/-90 ?
                i += 1
            elif command_next == "haut droite":
                order = "G X:0 Y:"+ str(avance)+" A:45"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-135"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas droite":
                order = "G X:0 Y:"+ str(avance)+" A:-45"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "tir":
                list_command2.append("tir")
                i += 1
            elif command_next == "etape":
                list_command2.append("RC D")
                list_command2.append("RC D")
                i += 1
            else:
                order = "G X:0 Y:"+ str(avance)+" A:0"
                list_command2.append(order)
                list_command2.append("S")
                i +=1
        elif command == "haut gauche":####################################"
            if command_next == "haut":
                order = "G X:0 Y:"+ str(avance)+" A:-45"
                list_command2.append(order)
                i += 1
            elif command_next == "bas":
                order = "G X:0 Y:"+ str(avance)+" A:135"
                list_command2.append(order)
                i += 1
            elif command_next == "gauche":
                order = "G X:0 Y:"+ str(avance)+" A:45"
                list_command2.append(order)
                i += 1
            elif command_next == "droite":
                order = "G X:0 Y:"+ str(avance)+" A:-135"
                list_command2.append(order) ### que 90/-90 ?
                i += 1
            elif command_next == "haut droite":
                order = "G X:0 Y:"+ str(avance)+" A:-90"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas gauche":
                order = "G X:0 Y:"+ str(avance)+" A:90"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas droite":
                order = "G X:0 Y:"+ str(avance)+" A:180"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "tir":
                list_command2.append("tir")
                i += 1
            elif command_next == "etape":
                list_command2.append("RC D")
                list_command2.append("RC D")
                i += 1
            else:
                order = "G X:0 Y:"+ str(avance)+" A:0"
                list_command2.append(order)
                list_command2.append("S")
                i+=1
        elif command == "haut droite": ###############################
            if command_next == "haut":
                order = "G X:0 Y:"+ str(avance)+" A:45"
                list_command2.append(order)
                i += 1
            elif command_next == "bas":
                order = "G X:0 Y:"+ str(avance)+" A:-135"
                list_command2.append(order)
                i += 1
            elif command_next == "gauche":
                order = "G X:0 Y:"+ str(avance)+" A:135"
                list_command2.append(order)
                i += 1
            elif command_next == "droite":
                order = "G X:0 Y:"+ str(avance)+" A:-45"
                list_command2.append(order) ### que 90/-90 ?
                i += 1
            elif command_next == "haut gauche":
                order = "G X:0 Y:"+ str(avance)+" A:90"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas gauche":
                order = "G X:0 Y:"+ str(avance)+" A:180"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas droite":
                order = "G X:0 Y:"+ str(avance)+" A:-90"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "tir":
                list_command2.append("tir")
                i += 1
            elif command_next == "etape":
                list_command2.append("RC D")
                list_command2.append("RC D")
                i += 1
            else:
                order = "G X:0 Y:"+ str(avance)+" A:0"
                list_command2.append(order)
                list_command2.append("S")
                i+=1
        elif command == "bas gauche":
            if command_next == "haut":
                order = "G X:0 Y:"+ str(avance)+" A:-135"
                list_command2.append(order)
                i += 1
            elif command_next == "bas":
                order = "G X:0 Y:"+ str(avance)+" A:45"
                list_command2.append(order)
                i += 1
            elif command_next == "gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-45"
                list_command2.append(order)
                i += 1
            elif command_next == "droite":
                order = "G X:0 Y:"+ str(avance)+" A:135"
                list_command2.append(order) ### que 90/-90 ?
                i += 1
            elif command_next == "haut droite":
                order = "G X:0 Y:"+ str(avance)+" A:180"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "haut gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-90"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas droite":
                order = "G X:0 Y:"+ str(avance)+" A:90"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "tir":
                list_command2.append("tir")
                i += 1
            elif command_next == "etape":
                list_command2.append("RC D")
                list_command2.append("RC D")
                i += 1
            else:
                order = "G X:0 Y:"+ str(avance)+" A:0"
                list_command2.append(order)
                list_command2.append("S")
                i+=1
        elif command == "bas droite":
            if command_next == "haut":
                order = "G X:0 Y:"+ str(avance)+" A:135"
                list_command2.append(order)
                i += 1
            elif command_next == "bas":
                order = "G X:0 Y:"+ str(avance)+" A:-45"
                list_command2.append(order)
                i += 1
            elif command_next == "gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-135"
                list_command2.append(order)
                i += 1
            elif command_next == "droite":
                order = "G X:0 Y:"+ str(avance)+" A:45"
                list_command2.append(order) ### que 90/-90 ?
                i += 1
            elif command_next == "haut droite":
                order = "G X:0 Y:"+ str(avance)+" A:90"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "haut gauche":
                order = "G X:0 Y:"+ str(avance)+" A:180"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "bas gauche":
                order = "G X:0 Y:"+ str(avance)+" A:-90"
                list_command2.append(order)### que 90/-90 ?
                i += 1
            elif command_next == "tir":
                list_command2.append("tir")
                i += 1
            elif command_next == "etape":
                list_command2.append("RC D")
                list_command2.append("RC D")
                i += 1
            else:
                order = "G X:0 Y:"+ str(avance)+" A:0"
                list_command2.append(order)
                list_command2.append("S")
                i+=1               
        else:

            i+=1
    
#    Creation du fichier texte avec les commandes
f =    open("parcours_optimal.txt","w")
for i in range(len(list_command2)):
    if list_command2[i] == "tir":
        exec(open("./calcul_pilotage_pointeur_lumineux.py").read())
        f =    open("parcours_optimal.txt","a")
    else:
        f.write("%s\r\n" %(list_command2[i]))
    
f.close() 

#                   
#                
#taille_robot = 45
#voisin = []
#for i in range(-taille_robot, taille_robot):
#        voisin.append((taille_robot,i))
#        voisin.append((-taille_robot,i))
#        voisin.append((i,taille_robot))
#        voisin.append((i,-taille_robot))
