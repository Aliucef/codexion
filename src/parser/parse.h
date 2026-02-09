/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 09:30:13 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/09 11:43:53 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSE_H
#define PARSE_H

#include "../validation/validate.h"

typedef struct s_parse
{
	int nb_of_coders;
	int time_to_burnout;
	int time_to_compile;
	int time_to_debug;
	int time_to_refactor;
	int required_compiles;
	int dongle_cooldown;
	char *scheduler;
} t_parse;

void init_arguments(t_parse *args, char **argv);

#endif
