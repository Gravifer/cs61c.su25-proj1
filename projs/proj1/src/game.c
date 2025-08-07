#include "game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_t *game, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_t *game, unsigned int snum);
static char next_square(game_t *game, unsigned int snum);
static void update_tail(game_t *game, unsigned int snum);
static void update_head(game_t *game, unsigned int snum);

/** Task 1
 * Create a default snake game in memory with the following starting game,
 * and return a pointer to the newly created game_t struct.
 * - The board has 18 rows, and each row has 20 columns.
 * - The fruit is at row 2, column 9 (zero-indexed).
 * - The tail is at row 2, column 2, and the head is at row 2, column 4.
 *  ####################
 *  #                  #
 *  # d>D    *         #
 *  #                  #
 *  ⋮        ⋮         ⋮
 *  #                  #
 *  ####################
 */
// static const int default_num_rows = 18, default_num_cols = 20;
enum default_dims {  //! static const int is only treated as a compile-time constant as an gcc extension
  default_num_rows = 18,
  default_num_cols = 20
};
/** The game->board is the board in memory, CONTAINING THE MUTATING GAMESTATE. 
 * Each element of the board array is a char* pointer 
 * to a character array containing a row of the board. 
 * The board is represented as an array of strings;
 * each row must be terminated by a new line and must be a valid string.
 * 
 * A game board is a grid of characters, not necessarily rectangular.
 * Here's an example of a non-rectangular board:
 *  ##############
 *  #            #######
 *  #####             ##
 *  #   #             ##
 *  #####             ######
 *  #                 ##   #
 *  #                 ######
 *  #                 ##
 *  #                  #
 *  #      #####       #
 *  ########   #########
 * Note that each row can have a different number of characters,
 * but will start and end with a wall (#).
 * It is assumed that the board is an enclosed space,
 * so snakes can't travel infinitely far in any direction.
 */
static const char default_board[default_num_rows][default_num_cols + 2] = {
    "####################\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "#                  #\n",
    "####################\n",
};
static const game_t default_game = {  //* pointer fields left NULL
    .num_rows = default_num_rows,
    .num_snakes = 1,
};
game_t *create_default_game() {
  // DONE: Implement this function.
  // Allocate game_t and board separately for better alignment
  game_t *game = malloc(sizeof(game_t));
  if (!game) {
    fprintf(stderr, "Failed to allocate memory for default game.\n");
    return NULL;
  }
  *game = default_game;

  // Allocate contiguous block for pointers + board data
  size_t s_ptrs = (default_num_rows * sizeof(char *)),
         s_board = sizeof(default_board);
  char *memory_block = malloc(s_ptrs + s_board);
  if (!memory_block) {
    fprintf(stderr, "Failed to allocate memory for default game board.\n");
    free(game);
    game = NULL;
    return NULL;
  }

  game->board = (char **)memory_block;
  char *board = memory_block + s_ptrs;

  // Set up row pointers
  for (size_t i = 0; i < game->num_rows; i++) {
    game->board[i] = board + i * (default_num_cols + 2);
  }

  // for (size_t i = 0; i < game->num_rows; i++) {
  //   strcpy(game->board[i], default_board[i]);
  // } //* the following memcpy is more efficient
  memcpy(board, default_board, sizeof(default_board));
  set_board_at(game, 2, 9, '*');  // fruit
  
  // Allocate and set up snakes
  game->snakes = malloc(sizeof(snake_t));
  if (!game->snakes) {
    fprintf(stderr, "Failed to allocate memory for snakes.\n");
      free(memory_block);
      memory_block = NULL;
      free(game);
      game = NULL;
      return NULL;
  }
  game->snakes[0] = (snake_t){
    .tail_row = 2, .tail_col = 2,
    .head_row = 2, .head_col = 4, .live = true
  };
  set_board_at(game, 2, 2, 'd');  // tail
  set_board_at(game, 2, 3, '>');  // body
  set_board_at(game, 2, 4, 'D');  // head
  return game;
}

/* Task 2 */
void free_game(game_t *game) {
  // DONE: Implement this function.
  //* All game instances MUST have their board contiguously allocated,
  //* so we can free the entire memory block in one go.
  if (!game) return;
  if (game->board) {
    /* Note: If each row was allocated separately, we would free them here.
     * But since we allocated it as part of a contiguous block, we free it all at once.
     * If you had allocated game->board separately, you would do:
    // for (unsigned int i = 0; i < game->num_rows; i++) {
    //   free(game->board[i]);  // Free each row if they were allocated separately
    // }
     */
    free(game->board);
    game->board = NULL;
  }
  if (game->snakes) {
    free(game->snakes);
    game->snakes = NULL;
  }
  free(game);
  game = NULL;
  return;
}

/* Task 3 */
void print_board(game_t *game, FILE *fp) {
  // TODO: Implement this function.
  return;
}

/*
  Saves the current game into filename. Does not modify the game object.
  (already implemented for you).
*/
void save_board(game_t *game, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(game, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_t *game, unsigned int row, unsigned int col) {
  return game->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_t *game, unsigned int row, unsigned int col, char ch) {
  game->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  // TODO: Implement this function.
  return true;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  return true;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  return true;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
  return '?';
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  // TODO: Implement this function.
  return '?';
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  // TODO: Implement this function.
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_game. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_t *game, unsigned int snum) {
  // TODO: Implement this function.
  return '?';
}

/*
  Task 4.3

  Helper function for update_game. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_t *game, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/*
  Task 4.4

  Helper function for update_game. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_t *game, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 4.5 */
void update_game(game_t *game, int (*add_food)(game_t *game)) {
  // TODO: Implement this function.
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  // TODO: Implement this function.
  return NULL;
}

/* Task 5.2 */
game_t *load_board(FILE *fp) {
  // TODO: Implement this function.
  return NULL;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_t *game, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 6.2 */
game_t *initialize_snakes(game_t *game) {
  // TODO: Implement this function.
  return NULL;
}
