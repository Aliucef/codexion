/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validate.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 11:33:16 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/22 16:09:02 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../parser/parse.h"

int	ft_strcmp(char *str, char *target)
{
	int	i;

	i = 0;
	while (str[i] && str[i] == target[i])
		i++;
	return (str[i] - target[i]);
}

int	is_num(char c)
{
	return (c >= '0' && c <= '9');
}

int	parse_scheduler(const char *str, t_sched *out)
{
	if (!str || !out)
		return (0);
	if (ft_strcmp((char *)str, "fifo") == 0)
		return (*out = SCED_FIFO, 1);
	if (ft_strcmp((char *)str, "edf") == 0)
		return (*out = SCED_EDF, 1);
	return (0);
}

int	is_valid(int argc, char **argv)
{
	int		i;
	int		j;
	t_sched	tmp;

	i = 1;
	if (argc != 9) // any number of args that is not 9 is rejected , including ./name
		return (0); // false
	while (i < 8) // iterating from 1 to 7 (parsing numerical arguments)
	{
		j = 0;
		while (argv[i][j])
		{
			if (!is_num(argv[i][j])) // if not number quit
				return (0);
			j++;
		}
		i++;
	}
	if (!parse_scheduler(argv[8], &tmp)) //check scheduler
		return (0);
	return (1);
}
