#ifndef UTILS_H
#define UTILS_H

#include "megaphone.h"

char	**ft_split(char const *s, char c);
int		ft_strlen(const char *str);
int		ft_atoi_base(char *str, char *base);
char	*to_hex_string(int number);
char	*ft_strjoin(char *s1, char *s2);
char	*getAvailableMulticastIPv6(void);

#endif
