/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validate.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 11:35:56 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/16 11:30:52 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VALIDATE_H
#define VALIDATE_H

#include "../parser/parse.h"

typedef enum e_sched
{
	SCED_FIFO,
	SCED_EDF
}	t_sched;


int	is_valid(int argc ,char **argv);
int	parse_scheduler(const char *str, t_sched *out);

#endif
