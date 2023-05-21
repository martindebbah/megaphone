# Projet Mégaphone PR6

## Auteurs

- AHMED YAHIA Yacine
- DEBBAH Martin
- ZHOU Alexandre

## Présentation

Le protocole Mégaphone est un protocole de communication de type client/serveur. Des utilisateurs se connectent à un serveur avec un client et peuvent ensuite suivre des fils de discussions, en
créer de nouveaux ou enrichir des fils existants en postant un nouveau message (billet), s’abonner
à un fil pour recevoir des notifications.
Plus précisément, un utilisateur peut s’inscrire auprès du serveur et une fois inscrit, il peut faire les actions suivantes :

– poster un message ou un fichier sur un nouveau fil,
– poster un message ou un fichier sur un fil existant,
– demander la liste des n derniers billets sur chaque fil,
– demander les n derniers billets d’un fil donné,
– s’abonner à un fil et recevoir les notifications de ce fil

## Exécuter le programme

Pour compiler le programme, se placer dans la racine du répertoire
puis lancer la commande

```bash
make
```

Pour exécuter le serveur, lancer la commande

```bash
./server
```

Pour exécuter un client, lancer la commande

```bash
./client <hostname>
```

Avec `hostname` la valeur affichée après l'exécution du serveur.

## Nettoyer le répertoire

Lancer la commande suivante pour supprimer tous fichiers compilés du répertoire

```bash
make clean
```
