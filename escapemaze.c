#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#define ROOM_W_OPENED 1
#define ROOM_N_OPENED 2
#define ROOM_E_OPENED 4
#define ROOM_S_OPENED 8
#define TYPE_CORNER '@'
#define TYPE_BORDER 'O'
#define TYPE_ROOM '+'
#define TYPE_CORRIDOR ' '
#define TYPE_WALL '#'
#define TYPE_EXIT 'X'
#define TYPE_PLAYER 'P'
#define TYPE_TROLLS 'T'
#define TYPE_HIDDEN '?'
#define CELL_HIDDEN 1
#define CELL_VISITED 2
#define DECISION_W 'w'
#define DECISION_N 'n'
#define DECISION_E 'e'
#define DECISION_S 's'
#define DECISION_REST 'r'
#define DECISION_QUIT 'q'

typedef struct room_s room_t;
typedef struct cell_s cell_t;

struct room_s {
	int mask;
	unsigned long rank;
	room_t *set;
	cell_t *cell;
};

struct cell_s {
	int type;
	room_t *room;
	unsigned long trolls_n;
	int mask;
	cell_t *from;
	unsigned long distance;
};

typedef struct {
	cell_t *last;
	cell_t *next;
	unsigned long n;
}
troll_move_t;

int enter_value(const char *, unsigned long, unsigned long, unsigned long *);
int escape_maze(unsigned long);
void init_room(room_t *);
room_t *find_set(room_t *);
void init_w_cell(room_t *);
void init_n_cell(room_t *);
void init_cell(cell_t *, int, room_t *);
void set_cell_visible(cell_t *);
void check_troll_move(cell_t *, cell_t *, cell_t *);
void check_link(cell_t *, cell_t *);
void add_to_queue(cell_t *, cell_t *, unsigned long);
void add_troll_move(cell_t *, cell_t *, unsigned long);
void init_troll_move(troll_move_t *, cell_t *, cell_t *, unsigned long);
void do_troll_move(troll_move_t *);
unsigned long erand(unsigned long);
int is_cell_accessible(cell_t *);
void free_data(void);

const char *decisions_room = "wnesWNESrq", *decisions_corridor = "wnesrq";
unsigned long view_rows_n, view_columns_n, row_rooms_n, column_rooms_n, rooms_n, column_walls_n, row_walls_n, walls_n, *walls = NULL, row_cells_n, column_cells_n, cells_n, switch_power, trolls_n, smell_power, troll_moves_n, q_cells_n;
room_t *rooms = NULL;
cell_t *cells = NULL, **q_cells = NULL;
troll_move_t *troll_moves = NULL;

int main(void) {
unsigned long rooms_n_max, q_cells_n_max, row_rooms_n_inc, column_rooms_n_inc, switch_power_inc, trolls_n_inc, smell_power_inc, level, *wall;
room_t *room;
cell_t *cell, **q_cell;
troll_move_t *troll_move;
	if (!enter_value("Number of rows in view", 3UL, ULONG_MAX, &view_rows_n)) {
		return EXIT_FAILURE;
	}
	if (!enter_value("Number of columns in view", 3UL, ULONG_MAX, &view_columns_n)) {
		return EXIT_FAILURE;
	}
	rooms_n_max = (ULONG_MAX-1)/2;
	if (!enter_value("Initial number of rooms by row", 2UL, rooms_n_max, &row_rooms_n)) {
		return EXIT_FAILURE;
	}
	if (!enter_value("Initial number of rooms by column", 2UL, rooms_n_max/row_rooms_n, &column_rooms_n)) {
		return EXIT_FAILURE;
	}
	rooms_n = row_rooms_n*column_rooms_n;
	rooms = malloc(sizeof(room_t)*rooms_n);
	if (!rooms) {
		fprintf(stderr, "Could not allocate memory for rooms\n");
		return EXIT_FAILURE;
	}
	column_walls_n = column_rooms_n-1;
	row_walls_n = row_rooms_n-1;
	walls_n = row_rooms_n*column_walls_n+column_rooms_n*row_walls_n;
	walls = malloc(sizeof(unsigned long)*walls_n);
	if (!walls) {
		fprintf(stderr, "Could not allocate memory for walls\n");
		free_data();
		return EXIT_FAILURE;
	}
	row_cells_n = row_rooms_n*2+1;
	column_cells_n = column_rooms_n*2+1;
	cells_n = row_cells_n*column_cells_n;
	cells = malloc(sizeof(cell_t)*cells_n);
	if (!cells) {
		fprintf(stderr, "Could not allocate memory for cells\n");
		free_data();
		return EXIT_FAILURE;
	}
	q_cells_n_max = (row_cells_n-2)*(column_cells_n-2);
	q_cells = malloc(sizeof(cell_t *)*q_cells_n_max);
	if (!q_cells) {
		fprintf(stderr, "Could not allocate memory for queue cells\n");
		free_data();
		return EXIT_FAILURE;
	}
	if (!enter_value("Initial switch power for player", 0UL, ULONG_MAX, &switch_power)) {
		free_data();
		return EXIT_FAILURE;
	}
	if (!enter_value("Initial number of trolls", 0UL, ULONG_MAX, &trolls_n)) {
		free_data();
		return EXIT_FAILURE;
	}
	troll_moves = malloc(sizeof(troll_move_t)*trolls_n);
	if (!troll_moves) {
		fprintf(stderr, "Could not allocate memory for troll moves\n");
		free_data();
		return EXIT_FAILURE;
	}
	if (!enter_value("Initial smell power for trolls", 0UL, ULONG_MAX, &smell_power)) {
		free_data();
		return EXIT_FAILURE;
	}
	if (!enter_value("Number of rooms by row added at each level (if possible)", 0UL, rooms_n_max, &row_rooms_n_inc)) {
		free_data();
		return EXIT_FAILURE;
	}
	if (!enter_value("Number of rooms by column added at each level (if possible)", 0UL, rooms_n_max/row_rooms_n_inc, &column_rooms_n_inc)) {
		free_data();
		return EXIT_FAILURE;
	}
	if (!enter_value("Switch power added at each level (if possible)", 0UL, ULONG_MAX, &switch_power_inc)) {
		free_data();
		return EXIT_FAILURE;
	}
	if (!enter_value("Number of trolls added at each level (if possible)", 0UL, ULONG_MAX, &trolls_n_inc)) {
		free_data();
		return EXIT_FAILURE;
	}
	if (!enter_value("Smell power added at each level (if possible)", 0UL, ULONG_MAX, &smell_power_inc)) {
		free_data();
		return EXIT_FAILURE;
	}
	srand((unsigned)time(NULL));
	level = 1;
	while (escape_maze(level) > 0) {
		if (row_rooms_n+row_rooms_n_inc <= rooms_n_max && row_rooms_n+row_rooms_n_inc >= row_rooms_n) {
			row_rooms_n += row_rooms_n_inc;
		}
		if (column_rooms_n+column_rooms_n_inc <= rooms_n_max/row_rooms_n && column_rooms_n+column_rooms_n_inc >= column_rooms_n) {
			column_rooms_n += column_rooms_n_inc;
		}
		rooms_n = row_rooms_n*column_rooms_n;
		room = realloc(rooms, sizeof(room_t)*rooms_n);
		if (!room) {
			fprintf(stderr, "Could not reallocate memory for rooms\n");
			free_data();
			return EXIT_FAILURE;
		}
		rooms = room;
		column_walls_n = column_rooms_n-1;
		row_walls_n = row_rooms_n-1;
		walls_n = row_rooms_n*column_walls_n+column_rooms_n*row_walls_n;
		wall = realloc(walls, sizeof(unsigned long)*walls_n);
		if (!wall) {
			fprintf(stderr, "Could not reallocate memory for walls\n");
			free_data();
			return EXIT_FAILURE;
		}
		walls = wall;
		row_cells_n = row_rooms_n*2+1;
		column_cells_n = column_rooms_n*2+1;
		cells_n = row_cells_n*column_cells_n;
		cell = realloc(cells, sizeof(cell_t)*cells_n);
		if (!cell) {
			fprintf(stderr, "Could not reallocate memory for cells\n");
			free_data();
			return EXIT_FAILURE;
		}
		cells = cell;
		q_cells_n_max = (row_cells_n-2)*(column_cells_n-2);
		q_cell = realloc(q_cells, sizeof(cell_t *)*q_cells_n_max);
		if (!q_cell) {
			fprintf(stderr, "Could not reallocate memory for queue cells\n");
			free_data();
			return EXIT_FAILURE;
		}
		q_cells = q_cell;
		if (switch_power+switch_power_inc >= switch_power) {
			switch_power += switch_power_inc;
		}
		if (trolls_n+trolls_n_inc >= trolls_n) {
			trolls_n += trolls_n_inc;
		}
		troll_move = realloc(troll_moves, sizeof(troll_move_t)*trolls_n);
		if (!troll_move) {
			fprintf(stderr, "Could not reallocate memory for troll moves\n");
			free_data();
			return EXIT_FAILURE;
		}
		troll_moves = troll_move;
		if (smell_power+smell_power_inc >= smell_power) {
			smell_power += smell_power_inc;
		}
		level++;
	}
	free_data();
	return EXIT_SUCCESS;
}

int enter_value(const char *name, unsigned long min, unsigned long max, unsigned long *value) {
	printf("%s [%lu-%lu] ? ", name, min, max);
	if (scanf("%lu", value) != 1 || *value < min || *value > max) {
		fprintf(stderr, "Value is invalid or out of range\n");
		return 0;
	}
	return 1;
}

int escape_maze(unsigned long level) {
const char *decisions;
int mask1, mask2, escaped, type, decision, direction;
unsigned long wall, room_row, trolls_n_cur, cell_column_min, cell_row_min, cell_column_max, cell_row_max, player_row, player_column, i, j;
room_t *room1, *room2, *set1, *set2;
cell_t *cell_exit, *player_last, *player_next, *cell;
	for (i = 0; i < rooms_n; i++) {
		init_room(rooms+i);
	}
	for (i = 0; i < walls_n; i++) {
		walls[i] = i;
	}
	j = 0;
	while (j < rooms_n-1) {
		wall = erand(i);
		if (walls[wall] < row_rooms_n*column_walls_n) {
			room_row = walls[wall]/column_walls_n;
			room1 = rooms+room_row*column_walls_n+walls[wall]%column_walls_n+room_row;
			room2 = room1+1;
			mask1 = ROOM_E_OPENED;
			mask2 = ROOM_W_OPENED;
		}
		else {
			room1 = rooms+walls[wall]/column_rooms_n*column_rooms_n+walls[wall]%column_rooms_n-row_rooms_n*column_walls_n;
			room2 = room1+column_rooms_n;
			mask1 = ROOM_S_OPENED;
			mask2 = ROOM_N_OPENED;
		}
		if (!(room1->mask & mask1)) {
			set1 = find_set(room1);
			set2 = find_set(room2);
			if (set1 != set2) {
				if (set1->rank < set2->rank) {
					set1->set = set2;
				}
				else if (set1->rank > set2->rank) {
					set2->set = set1;
				}
				else {
					set1->rank++;
					set2->set = set1;
				}
				room1->mask += mask1;
				room2->mask += mask2;
				j++;
			}
		}
		walls[wall] = walls[--i];
	}
	room1 = rooms;
	init_cell(cells, TYPE_CORNER, NULL);
	cell = cells+1;
	for (i = 1; i < column_cells_n-1; i++) {
		init_cell(cell, TYPE_BORDER, NULL);
		cell++;
	}
	init_cell(cell, TYPE_CORNER, NULL);
	cell++;
	for (i = 1; i < row_cells_n-1; i++) {
		init_cell(cell, TYPE_BORDER, NULL);
		cell++;
		if (i%2) {
			for (j = 1; j < column_cells_n-2; j += 2) {
				init_cell(cell, TYPE_ROOM, room1);
				room1->cell = cell;
				room1++;
				cell += 2;
			}
			init_cell(cell, TYPE_ROOM, room1);
			room1->cell = cell;
			room1++;
			cell++;
		}
		else {
			cell++;
			for (j = 2; j < column_cells_n-1; j += 2) {
				init_cell(cell, TYPE_WALL, NULL);
				cell += 2;
			}
		}
		init_cell(cell, TYPE_BORDER, NULL);
		cell++;
	}
	init_cell(cell, TYPE_CORNER, NULL);
	cell++;
	for (i = 1; i < column_cells_n-1; i++) {
		init_cell(cell, TYPE_BORDER, NULL);
		cell++;
	}
	init_cell(cell, TYPE_CORNER, NULL);
	room1 = rooms+1;
	for (i = 1; i < column_rooms_n; i++) {
		init_w_cell(room1);
		room1++;
	}
	for (i = 1; i < row_rooms_n; i++) {
		init_n_cell(room1);
		room1++;
		for (j = 1; j < column_rooms_n; j++) {
			init_w_cell(room1);
			init_n_cell(room1);
			room1++;
		}
	}
	do {
		cell_exit = cells+erand(cells_n);
	}
	while (cell_exit->type != TYPE_BORDER);
	do {
		player_last = cells+erand(cells_n);
	}
	while (!is_cell_accessible(player_last));
	for (i = 0; i < trolls_n; i++) {
		do {
			cell = cells+erand(cells_n);
		}
		while (!is_cell_accessible(cell) || cell == player_last);
		cell->trolls_n++;
	}
	trolls_n_cur = trolls_n;
	for (i = 0; i < cells_n; i++) {
		cells[i].mask = CELL_HIDDEN;
	}
	set_cell_visible(player_last);
	escaped = 0;
	do {
		if (player_last != cell_exit && !player_last->trolls_n) {
			cell = player_last;
			do {
				cell--;
				set_cell_visible(cell+column_cells_n);
				set_cell_visible(cell);
				set_cell_visible(cell-column_cells_n);
			}
			while (is_cell_accessible(cell) && !cell->trolls_n);
			cell = player_last;
			do {
				cell -= column_cells_n;
				set_cell_visible(cell-1);
				set_cell_visible(cell);
				set_cell_visible(cell+1);
			}
			while (is_cell_accessible(cell) && !cell->trolls_n);
			cell = player_last;
			do {
				cell++;
				set_cell_visible(cell-column_cells_n);
				set_cell_visible(cell);
				set_cell_visible(cell+column_cells_n);
			}
			while (is_cell_accessible(cell) && !cell->trolls_n);
			cell = player_last;
			do {
				cell += column_cells_n;
				set_cell_visible(cell+1);
				set_cell_visible(cell);
				set_cell_visible(cell-1);
			}
			while (is_cell_accessible(cell) && !cell->trolls_n);
		}
		cell = cells+cells_n-column_cells_n;
		for (cell_column_min = 0; cell_column_min < column_cells_n; cell_column_min++) {
			for (i = row_cells_n; i && (cell->mask & CELL_HIDDEN); i--) {
				cell -= column_cells_n;
			}
			if (i) {
				break;
			}
			else {
				cell += cells_n+1;
			}
		}
		cell = cells;
		for (cell_row_min = 0; cell_row_min < row_cells_n; cell_row_min++) {
			for (i = 0; i < column_cells_n && (cell->mask & CELL_HIDDEN); i++) {
				cell++;
			}
			if (i < column_cells_n) {
				break;
			}
		}
		cell = cells+column_cells_n-1;
		for (cell_column_max = column_cells_n; cell_column_max; cell_column_max--) {
			for (i = 0; i < row_cells_n && (cell->mask & CELL_HIDDEN); i++) {
				cell += column_cells_n;
			}
			if (i < row_cells_n) {
				break;
			}
			else {
				cell -= cells_n+1;
			}
		}
		cell = cells+cells_n-1;
		for (cell_row_max = row_cells_n; cell_row_max; cell_row_max--) {
			for (i = column_cells_n; i && (cell->mask & CELL_HIDDEN); i--) {
				cell--;
			}
			if (i) {
				break;
			}
		}
		player_row = 0;
		for (cell = cells+column_cells_n; cell < player_last; cell += column_cells_n) {
			player_row++;
		}
		player_column = 0;
		for (cell = cells+column_cells_n*player_row; cell < player_last; cell++) {
			player_column++;
		}
		while (cell_row_max-cell_row_min > view_rows_n) {
			if (player_row-cell_row_min > cell_row_max-player_row) {
				cell_row_min++;
			}
			else {
				cell_row_max--;
			}
		}
		while (cell_column_max-cell_column_min > view_columns_n) {
			if (player_column-cell_column_min > cell_column_max-player_column) {
				cell_column_min++;
			}
			else {
				cell_column_max--;
			}
		}
		putchar('\n');
		cell = cells+cell_row_min*column_cells_n;
		for (i = cell_row_min; i < cell_row_max; i++) {
			cell += cell_column_min;
			for (j = cell_column_min; j < cell_column_max; j++) {
				type = cell->type;
				if (cell == cell_exit) {
					type = TYPE_EXIT;
				}
				if (cell == player_last) {
					type = TYPE_PLAYER;
				}
				if (cell->trolls_n) {
					type = TYPE_TROLLS;
				}
				if (cell->mask & CELL_HIDDEN) {
					type = TYPE_HIDDEN;
				}
				putchar(type);
				cell++;
			}
			putchar('\n');
			cell += column_cells_n-cell_column_max;
		}
		printf("\nLevel %lu  Rooms %lux%lu  Switch power %lu  Trolls %lu (Smell power %lu)\n", level, row_rooms_n, column_rooms_n, switch_power, trolls_n_cur, smell_power);
		if (player_last == cell_exit) {
			printf("You found the exit and go to next level !\n");
			escaped = 1;
		}
		else if (player_last->trolls_n) {
			printf("Game over !\n");
			escaped = -1;
		}
		else {
			decisions = player_last->type == TYPE_ROOM && switch_power ? decisions_room:decisions_corridor;
			do {
				printf("Your decision [%s] ? ", decisions);
				do {
					decision = fgetc(stdin);
				}
				while (decision == '\n');
			}
			while (!strchr(decisions, decision));
			if (decision == DECISION_QUIT) {
				escaped = -2;
			}
			else {
				direction = tolower(decision);
				if (direction == DECISION_W) {
					player_next = player_last-1;
				}
				else if (direction == DECISION_N) {
					player_next = player_last-column_cells_n;
				}
				else if (direction == DECISION_E) {
					player_next = player_last+1;
				}
				else if (direction == DECISION_S) {
					player_next = player_last+column_cells_n;
				}
				else {
					player_next = player_last;
				}
				troll_moves_n = 0;
				for (i = 0; i < cells_n; i++) {
					check_troll_move(cells+i, player_last, player_next);
				}
				if (direction == decision) {
					if (is_cell_accessible(player_next) || player_next == cell_exit) {
						player_last = player_next;
					}
				}
				else {
					if (player_next->type == TYPE_CORRIDOR) {
						switch_power--;
						trolls_n_cur -= player_next->trolls_n;
						player_next->type = TYPE_WALL;
						player_next->trolls_n = 0;
					}
					else if (player_next->type == TYPE_WALL) {
						switch_power--;
						player_next->type = TYPE_CORRIDOR;
					}
				}
				for (i = 0; i < troll_moves_n; i++) {
					do_troll_move(troll_moves+i);
				}
			}
		}
	}
	while (!escaped);
	return escaped;
}

void init_room(room_t *room) {
	room->mask = 0;
	room->rank = 0;
	room->set = room;
}

room_t *find_set(room_t *room) {
	if (room->set != room) {
		room->set = find_set(room->set);
	}
	return room->set;
}

void init_w_cell(room_t *room) {
	init_cell(room->cell-1, room->mask & ROOM_W_OPENED ? TYPE_CORRIDOR:TYPE_WALL, NULL);
}

void init_n_cell(room_t *room) {
	init_cell(room->cell-column_cells_n, room->mask & ROOM_N_OPENED ? TYPE_CORRIDOR:TYPE_WALL, NULL);
}

void init_cell(cell_t *cell, int type, room_t *room) {
	cell->type = type;
	cell->room = room;
	cell->trolls_n = 0;
}

void set_cell_visible(cell_t *cell) {
	if (cell->mask & CELL_HIDDEN) {
		cell->mask -= CELL_HIDDEN;
	}
}

void check_troll_move(cell_t *last, cell_t *player_last, cell_t *player_next) {
int found;
unsigned long i, j, k;
cell_t *cell_w, *cell_n, *cell_e, *cell_s, *cell;
	if (last != player_next && last->trolls_n) {
		cell_w = last;
		do {
			cell_w--;
		}
		while (is_cell_accessible(cell_w) && cell_w != player_last);
		cell_n = last;
		if (cell_w != player_last) {
			do {
				cell_n -= column_cells_n;
			}
			while (is_cell_accessible(cell_n) && cell_n != player_last);
		}
		cell_e = last;
		if (cell_w != player_last && cell_n != player_last) {
			do {
				cell_e++;
			}
			while (is_cell_accessible(cell_e) && cell_e != player_last);
		}
		cell_s = last;
		if (cell_w != player_last && cell_n != player_last && cell_e != player_last) {
			do {
				cell_s += column_cells_n;
			}
			while (is_cell_accessible(cell_s) && cell_s != player_last);
		}
		if (cell_w == player_last) {
			add_troll_move(last, last-1, last->trolls_n);
		}
		else if (cell_n == player_last) {
			add_troll_move(last, last-column_cells_n, last->trolls_n);
		}
		else if (cell_e == player_last) {
			add_troll_move(last, last+1, last->trolls_n);
		}
		else if (cell_s == player_last) {
			add_troll_move(last, last+column_cells_n, last->trolls_n);
		}
		else {
			q_cells_n = 0;
			add_to_queue(last, NULL, 0UL);
			for (i = 0; i < q_cells_n && q_cells[i] != player_last && q_cells[i]->distance <= smell_power; i++) {
				check_link(q_cells[i], q_cells[i]-1);
				check_link(q_cells[i], q_cells[i]-column_cells_n);
				check_link(q_cells[i], q_cells[i]+1);
				check_link(q_cells[i], q_cells[i]+column_cells_n);
			}
			found = i < q_cells_n && q_cells[i]->distance <= smell_power;
			if (found) {
				for (cell = q_cells[i]; cell->distance > 1; cell = cell->from);
				add_troll_move(last, cell, last->trolls_n);
			}
			for (i = 0; i < q_cells_n; i++) {
				q_cells[i]->mask -= CELL_VISITED;
			}
			if (!found) {
				q_cells_n = 0;
				add_to_queue(last, NULL, 0UL);
				for (i = 0; i < q_cells_n && q_cells[i]->distance < 2; i++) {
					check_link(q_cells[i], q_cells[i]-1);
					check_link(q_cells[i], q_cells[i]-column_cells_n);
					check_link(q_cells[i], q_cells[i]+1);
					check_link(q_cells[i], q_cells[i]+column_cells_n);
				}
				if (i > 1) {
					for (j = 0; j < last->trolls_n; j++) {
						k = erand(i);
						if (k) {
							add_troll_move(last, q_cells[k], 1UL);
						}
					}
				}
				for (i = 0; i < q_cells_n; i++) {
					q_cells[i]->mask -= CELL_VISITED;
				}
			}
		}
	}
}

void check_link(cell_t *from, cell_t *to) {
	if (is_cell_accessible(to) && !(to->mask & CELL_VISITED)) {
		add_to_queue(to, from, from->distance+1);
	}
}

void add_to_queue(cell_t *cell, cell_t *from, unsigned long distance) {
	cell->mask += CELL_VISITED;
	cell->from = from;
	cell->distance = distance;
	q_cells[q_cells_n++] = cell;
}

void add_troll_move(cell_t *last, cell_t *next, unsigned long n) {
	init_troll_move(troll_moves+troll_moves_n, last, next, n);
	troll_moves_n++;
}

void init_troll_move(troll_move_t *troll_move, cell_t *last, cell_t *next, unsigned long n) {
	troll_move->last = last;
	troll_move->next = next;
	troll_move->n = n;
}

void do_troll_move(troll_move_t *troll_move) {
	if (is_cell_accessible(troll_move->next)) {
		troll_move->last->trolls_n -= troll_move->n;
		troll_move->next->trolls_n += troll_move->n;
	}
}

unsigned long erand(unsigned long values) {
	return (unsigned long)(rand()/(RAND_MAX+1.0)*values);
}

int is_cell_accessible(cell_t *cell) {
	return cell->type == TYPE_ROOM || cell->type == TYPE_CORRIDOR;
}

void free_data(void) {
	if (troll_moves) {
		free(troll_moves);
	}
	if (q_cells) {
		free(q_cells);
	}
	if (cells) {
		free(cells);
	}
	if (walls) {
		free(walls);
	}
	if (rooms) {
		free(rooms);
	}
}
