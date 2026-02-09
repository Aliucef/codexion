/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validate.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 11:33:16 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/09 12:07:24 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

int	valid_scheduler(char *str)
{
	return (ft_strcmp(str, "fifo") == 0 || ft_strcmp(str, "edf") == 0);
}

int	is_valid(int argc, char **argv)
{
	int	i;
	int	j;

	i = 1;
	if (argc != 9)
		return (0);
	while (i < 8)
	{
		j = 0;
		while (argv[i][j])
		{
			if (!is_num(argv[i][j]))
				return (0);
			j++;
		}
		i++;
	}
	if (!valid_scheduler(argv[i]))
		return (0);
	return (1);
}
