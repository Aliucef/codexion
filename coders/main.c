/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 11:42:11 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/09 12:04:59 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/parse.h"
#include "stdio.h"

int	main(int argc, char **argv)
{
	t_parse	args;

	if (!is_valid(argc, argv))
	{
		printf("fail parameters\n");
		return (0);
	}
	init_arguments(&args, argv);
}
