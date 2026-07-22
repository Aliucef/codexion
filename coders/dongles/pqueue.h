/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pqueue.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 00:00:00 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/22 00:00:00 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PQUEUE_H
# define PQUEUE_H

# include "../coders.h"

void	pq_sift_up(t_dongle *d, int i);
void	pq_sift_down(t_dongle *d, int i);
void	pq_push(t_dongle *d, int coder_id, long priority);
int		pq_pop(t_dongle *d);
int		pq_peek(t_dongle *d);
void	pq_remove(t_dongle *d, int coder_id);

#endif
