#ifndef UTILS_H
#define UTILS_H

#include "megaphone.h"

// Découpe une chaîne de caractères en fonction du token
char **ft_split(char const *s, char token);

// Convertit une chaîne de caractères en sa représentation selon la base
int ft_atoi_base(char *str, char *base);

// Convertit un entier en sa représentation hexadécimale sous forme de chaîne de caractères
char *to_hex_string(int number);

// Concatène s1 et s2
char *ft_strjoin(char *s1, char *s2);

// Renvoie une adresse IPv6 disponible pour le muticast
char *getAvailableMulticastIPv6(void);

#endif
