#include "../include/utils.h"

static int	count_word(char const *s, char c)
{
	int	i;
	int	res;

	i = 0;
	res = 0;
	while (s[i])
	{
		while (s[i] == c && s[i])
			i++;
		if (s[i] != c && s[i])
		{
			res++;
			while (s[i] != c && s[i])
				i++;
		}
	}
	return (res);
}

static char	*get_word(char const *s, char c, int *index)
{
	int		i;
	int		len;
	char	*res;

	i = 0;
	len = 0;
	while (s[(*index)] == c && s[(*index)])
		(*index)++;
	while (s[(*index) + len] != c && s[(*index) + len])
		len++;
	res = malloc(sizeof(char) * (len + 1));
	if (!res)
		return (NULL);
	while (i < len)
	{
		res[i] = s[(*index)];
		i++;
		(*index)++;
	}
	res[i] = '\0';
	return (res);
}

char	**ft_split(char const *s, char c)
{
	int		i;
	int		j;
	char	**res;

	i = 0;
	j = 0;
	res = malloc(sizeof(char *) * (count_word(s, c) + 1));
	if (!res)
		return (NULL);
	while (i < count_word(s, c))
	{
		res[i] = get_word(s, c, &j);
		if (!res[i])
		{
			while (i >= 0)
			{
				free(res[i]);
				i--;
			}
			return (NULL);
		}
		i++;
	}
	res[i] = 0;
	return (res);
}


int	ft_strlen(const char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

int	check_base(char *base)
{
	int	i;
	int	j;

	i = -1;
	if (ft_strlen(base) == 1 || base[0] == '\0')
		return (1);
	while (base[++i])
	{
		if (base[i] < 32 || base[i] == 127
			|| base [i] == '-' || base[i] == '+')
			return (1);
	}
	i = 0;
	while (i < ft_strlen(base))
	{
		j = i + 1;
		while (j < ft_strlen(base))
		{
			if (base[i] == base[j])
				return (1);
			j++;
		}
		i++;
	}
	return (0);
}

int	power_to(int n, int p)
{
	if (p == 0)
		return (1);
	return (n * (power_to(n, p - 1)));
}

int	convert(char c, char *base)
{
	int	i;

	i = 0;
	while (base[i])
	{
		if (base[i] == c)
			return (i);
		i++;
	}
	return (-1);
}

int	ft_atoi_base(char *str, char *base)
{
	int	res;
	int	i;
	int	pow;

	if (str == NULL || base == NULL || check_base(base) == 1)
		return (0);
	res = 0;
	i = 0;
	while ((str[i] >= 9 && str[i] <= 13) || str[i] == ' ')
		i++;
	pow = ft_strlen(str) - i - 1;
	while (str[i] && convert(str[i], base) != (-1))
	{
		res += convert(str[i], base) * power_to(ft_strlen(base), pow--);
		i++;
	}
	return (res);
}

char	*to_hex_string(int number) {
    char* result = malloc(sizeof(char) * (sizeof(int) * 2 + 1));
    sprintf(result, "%X", number);
    return result;
}


char	*ft_strjoin(char *s1, char *s2) {
	int		i;
	int		j;
	char	*str;

	i = -1;
	j = 0;
	if (!s1 || s1[0] == '\0')
		s1 = calloc(1, sizeof(char));
	if (!s1 || !s2)
		return (NULL);
	str = calloc((ft_strlen((const char*)s1) + ft_strlen((const char*)s2) + 1), sizeof(char));
	if (str == NULL)
		return (NULL);
	while (s1[++i])
		str[i] = s1[i];
	while (s2[j])
		str[i++] = s2[j++];
	free(s1);
	return (str);
}

char* getAvailableMulticastIPv6(void) {
    char* multicastAddr = malloc(INET6_ADDRSTRLEN);
    if (multicastAddr == NULL) {
        perror("Erreur lors de l'allocation de mémoire");
        return NULL;
    }

    struct in6_addr minAddr, maxAddr, randomAddr;
    inet_pton(AF_INET6, "FF00::", &minAddr);
    inet_pton(AF_INET6, "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF", &maxAddr);

    srand(time(NULL));
    do {
        for (int i = 0; i < 16; i++) {
            unsigned int randomPart = rand() % 65536; // 2^16
			randomAddr.__in6_u.__u6_addr16[i] = htons(randomPart);
			// randomAddr.__u6_addr.__u6_addr16[i] = htons(randomPart);
        }
    } while (memcmp(&randomAddr, &minAddr, sizeof(struct in6_addr)) < 0 ||
             memcmp(&randomAddr, &maxAddr, sizeof(struct in6_addr)) > 0);

    if (inet_ntop(AF_INET6, &randomAddr, multicastAddr, INET6_ADDRSTRLEN) == NULL) {
        perror("Erreur lors de la conversion de l'adresse en chaîne");
        return NULL;
    }

    return multicastAddr;
}
