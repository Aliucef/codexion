/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validate.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 11:33:16 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:08:48 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../parser/parse.h" // t_sched, SCED_FIFO, SCED_EDF

// Custom strcmp: compares two C strings character by character.
// Returns 0 if equal, non-zero (positive/negative) on first difference.
// Example: ft_strcmp("fifo", "fifo") → scans f,i,f,o all match → returns 0.
// Example: ft_strcmp("edf", "fifo") → 'e'-'f' = -1 → returns -1.
int	ft_strcmp(char *str, char *target)
{
	int	i; // index into both strings

	i = 0;
	while (str[i] && str[i] == target[i]) // advance while characters match and str not ended
		i++;
	return (str[i] - target[i]); // e.g. both '\0' → 0-0=0 (equal); 'e'-'f' → -1 (str < target)
}

// Returns 1 if character c is a digit ('0'–'9'), 0 otherwise.
// Used to validate that numeric arguments contain only digit characters.
// Example: is_num('5') → 1; is_num('-') → 0; is_num('a') → 0.
int	is_num(char c)
{
	return (c >= '0' && c <= '9'); // ASCII range check: '0'=48, '9'=57
}

// Parses the scheduler string into the t_sched enum and writes it to *out.
// Returns 1 on success, 0 if str is not "fifo" or "edf".
// Example: parse_scheduler("fifo", &args.scheduler) → *out = SCED_FIFO, return 1.
// Example: parse_scheduler("rr",   &args.scheduler) → return 0 (unknown scheduler).
int	parse_scheduler(const char *str, t_sched *out)
{
	if (!str || !out)              // null guard — shouldn't happen but safe to check
		return (0);
	if (ft_strcmp((char *)str, "fifo") == 0) // "fifo" matches → FIFO scheduler
		return (*out = SCED_FIFO, 1);        // set enum value and return success
	if (ft_strcmp((char *)str, "edf") == 0)  // "edf" matches → EDF scheduler
		return (*out = SCED_EDF, 1);         // set enum value and return success
	return (0); // unknown string, e.g. "rr" or "lifo" → validation fails
}

// Validates all 8 command-line arguments before any parsing happens.
// Returns 1 if valid, 0 if anything is wrong.
// Checks: exactly 9 args (program + 8), args 1–7 are all-digit strings, arg 8 is "fifo"/"edf".
// Example valid:   ./codexion 5 800 200 200 200 3 50 fifo → returns 1.
// Example invalid: ./codexion 5 800 200 200 200 3 50      → argc=8, not 9 → returns 0.
// Example invalid: ./codexion 5 800 -200 200 200 3 50 fifo → '-' is not a digit → returns 0.
int	is_valid(int argc, char **argv)
{
	int		i; // argument index (1–7 for numeric args)
	int		j; // character index within each argument string
	t_sched	tmp; // throwaway scheduler enum just to validate arg 8

	i = 1;
	if (argc != 9)  // must have exactly 8 arguments + program name, e.g. argc=9
		return (0); // wrong number of args → fail immediately
	while (i < 8)  // check argv[1] through argv[7] (the numeric args)
	{
		j = 0;
		while (argv[i][j]) // scan each character of this argument
		{
			if (!is_num(argv[i][j])) // found a non-digit, e.g. '-', '.', 'x'
				return (0);          // invalid numeric argument → fail
			j++;
		}
		i++;
	}
	if (!parse_scheduler(argv[8], &tmp)) // validate arg 8 is "fifo" or "edf"
		return (0); // unknown scheduler string → fail
	return (1); // all checks passed → arguments are valid
}
