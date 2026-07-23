/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 13:12:15 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:12:15 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>        // printf()
#include "../parser/parse.h" // t_sim, get_time_ms(), sim->log_m, sim->start_ms

// Prints a timestamped status line for a coder to stdout.
// Format: "<elapsed_ms> <coder_id> <message>"
// Example output: "742 3 is compiling"
//   742ms elapsed since sim start, coder 3, doing "is compiling".
// log_m serializes all printf calls — without it, two threads could interleave
// their output and produce garbled lines like "742 3 is co742 1 mpiling".
void	log_state(t_sim *sim, int coder_id, const char *message)
{
	long	now;       // absolute time in ms, e.g. 1742
	long	timestamp; // time since simulation started, e.g. 742ms (if start_ms=1000)

	if (!sim || !message) // guard against null pointers (shouldn't happen in normal flow)
		return ;
	now = get_time_ms();             // e.g. 1742ms (absolute wall-clock time)
	timestamp = now - sim->start_ms; // e.g. 1742 - 1000 = 742ms since the sim began
	pthread_mutex_lock(&sim->log_m); // lock stdout — only one thread prints at a time
	printf("%ld %d %s\n", timestamp, coder_id, message); // e.g. "742 3 is compiling\n"
	pthread_mutex_unlock(&sim->log_m); // release — next thread may now print
}
