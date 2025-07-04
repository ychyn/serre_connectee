# serre_connectee

   a) une description courte et synthétique de votre projet tel que vous l'avez compris et réalisé
   b) la description de vos choix techniques
   c) les outils et les librairies, avec leur version, que nous devons utiliser pour exécuter vos projets
   d) le numéro (hash) du commit contenant la version stable de votre projet

Le but de ce projet était de construire une serre connectée.
Nous sommes parvenus au terme de cette semaine à pouvoir mesurer la température, la luminosité, l'humidité de l'air et celle du sol dans la serre et à agir sur son irrigation, sa ventilation et sa luminosité, le tout en partageant toutes les grandeurs physiques dans un fichier csv. Toutes les grandeurs étaient dans le même temps affichées sur un écran lcd, mis à jour toutes les 3 secondes.

Nous avons utilisé pour l'électronique 1 carte ESP32, 1 carte ESP32S2, 1 capteur BME280 (température, pression, humidité), 1 photo-résistance, 2 capteurs 'maker soil moisture sensor' de chez cytron, 1 écran LCD, 4 lampes UV, 1 relais, des résistances, 1 potentiomètres, 1 servo, 1 alimentation 12V externe aux cartes, 2 piles de 3.6V et 1 stabilisateur.
En parallèle, nous avons utilisé pour la structure du bois, du PVC les deux coupés au laser et des éléments plastiques imprimés en 3D.

Notre travail était :
1- informatique 
2- manuel.


1-
Nous avons commencé par prendre en main les différents capteurs et leurs branchements sur une carte arduino MEGA, n'ayant pas encore reçu les cartes ESP32. Nous avons modélisé les branchements sur tinkerCAD mais nous sommes vite passés à des branchements physiques car le logiciel ne disposait pas des mêmes capteurs que nous. Pour cela nous avons codé en C++ sur Arduino IDE en important différentes librairies selon la carte et les capteurs utilisés. 
Nous avons rencontrés de nombreux problèmes liés au branchement avec la carte ESP32 lorsque nous l'avons reçue car il fallait installer un driver afin de pouvoir l'utiliser et aucun de nous n'en avait déjà utilisé.
Notre première étape a été d'étalonner les capteurs d'humidité du sol : humidité maximale dans un verre d'eau et minimale à l'air et codage d'une "map" afin de normaliser les valeurs entre 0 et 100%.
Notre deuxième objectif était d'afficher des informations sur l'écran LCD ce qui nécessitait de nombreux branchements. Nous avons commencé par afficher la date et l'heure, récupérée sur un serveur grâce à une librairie de l'ESP32. Nous avons ensuite voulu afficher à la place les données renvoyées par les capteurs : humidité de l'air, du sol et la température. 
A cause d'une erreur de code nous avons pendant longtemps eu du mal à récupérer les valeurs renvoyées par le BME280 récupérant du NaN pendant au moins 2 jours.

Ensuite, nous nous sommes chacun intéressés à une fonctionnalité différente : éclairage, serveur pour stocker des données CSV, ventilateur, code global et irrigation.


Ventilateur : le but était de commander le ventilateur en fonction de l'humidité de l'air. Ainsi, nous pensions premièrement à utiliser une sortie PWM de l'ESP32, pour faire varier la vitesse du ventilateur en fonction de l'humidité (si très sec : vitesse max) mais c'était compliqué. Nous avons donc utilisé un relais pour pouvoir l'allumer ou l'éteindre à partir d'un certain seuil. Finalement, étant donné que c'est plus facile de faire varier l'humidité du sol, nous avons commandé ce relais à partir de la valeur renvoyée par ce capteur ci. Un problème s'est posé cependant : le ventilateur doit être alimenté par du 12V ce qui n'existe pas sur la carte ESP32 donc nous avons utilisé une alimentation externe et un stabilisateur 12->5V pour faire la liasion ventilateur->relais+lumière (voir plus bas). 


Irrigation : ESP32servo qui permet à l'aide de la commande servo.write d'indiquer un angle au servo, ici commandé en fonction de l'humidité du sol.


Récupération des données : Pour le stockage de données recuillies par nos capteurs, étant donnée que la carte ESP32 dont on disposait ne possédait pas de mémoire suffisante pour le stockage, on a décidé d'utiliser les fonctionnalités de Wifi présentes sur ce modèle. On a donc hébergé un site web sur notre microcontroleur qui affiche un fichier JSON, et qui est disponible sur tous les dispositifs connectés au même réseau wifi que la puce. Ensuite, via un code python dans notre ordinateur on récupère ces données et les stocke sur un fichier csv, qui nous permettrait éventuellement de faire un traitement de ces données.


Lumière : Nous avons ensuite équipé la serre d'un éclairage automatisé. Pou cela nous avons utilisé des Leds ZIP. Chacun des anneaux des anneaux de Leds fournis se branche sur trois fils : deux pour l'alimentation 5v et un pour commander l'éclairage. Chacune de ces leds est dotée de 3 cellules RGB. Le troisième fil permet de commander exactement la valeur RGB de l'éclairage. Le codage se fait grace  à la librairie adafruit_neopixel. Pour le cablage, nous avons réalisé un circuit d'anneaux de leds en parallèle au poste à souder du fablab. Etant donné que nous souhaitions un éclairage homogène de la serre, nous avons pu utiliser un fil commun pour envoyer la valeur RGB. 
Une fois l'éclairage fonctionnel, nous l'avons asservi par rapport à l'ensoleillement de la serre. Pour cela nous avons utilisé une photodiode en guise de capteur de luminosité. La résistance de cette diode diminue avec l'ensoleillement. Nous avons donc ajouté au code Arduino une condition sur la valeur minimale de la photorésistance qui déclenche l'éclairage de la serre.


2-
Très vite nous nous sommes également intéressés en parallèle à la structure physique et avons choisis un exemple de serre à créer soi-même sur internet. Cependant, les dimensions n'étaient pas adaptées car nous voulions une serre plus grande et avions des contraintes phsyiques sur la largeur des plaques de bois disponibles au FabLab. Ainsi, nous avons tout redimensionné à la main sur Fusion360 avant de lancer la découpe laser. Bien que la serre se montait et se tenait déjà relativement bien nous avons préféré rajouter des équerres imprimées en 3D à l'intérieur afin de consolider le tout. Il a fallu revenir à certaines reprises sur la structure afin d'incorporer chaque fonctionnalité petit à petit : impression 3D d'un support pour la réserve d'eau et forage d'un trou pour faire passer le tuyau, trou pour le ventilateur... 
Nous étions censés recevoir des valves à eau le deuxième jour mais étant donné qu'on n'était finalement pas sûrs de les recevoir à temps, nous avons alors créé notre propre système de commande de l'irrigation : utilisation d'un servo moteur intégré à un système imprimé 3D afin de pincer ou non un tuyau relié à la réserve d'eau en fonction de l'humidité du sol.



CODE : 
Nous n'avons pas pu tout centraliser en 1 seul code car ça aurait impliqué de travailler avec 1 seule carte et donc de tester les fontionnalités 1 par 1 or nous n'avions pas énormément de temps et les alimentations à utiliser n'avaient pas le même voltage. Nous avons donc utilisé 2 cartes et écrit 3 codes en parallèle que nous n'avons pas eu le temps de combiner.
Les 3 codes sont : le ventilateur seul, les lumières seules, le reste (irrigation, écran, récupération des données).


Toutes les librairies utilisées : 
Adafruit BME280 Library

Adafruit Unified Sensor

ArduinoJson

Async TCP

BME280

ESP Async WebServer

ESP32 Servo