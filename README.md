# EscapeMaze
Solution to challenge https://www.reddit.com/r/dailyprogrammer/comments/4vrb8n/weekly_25_escape_the_trolls/

Find the exit in a perfect maze full of hungry trolls.

It is a multi-level game, when you find the exit at one level you will go to the next.

The player will be able to switch a wall to a corridor and vice-versa, it is called below the "switch power".

If a cell is switched to a wall with trolls inside, all trolls are crushed.

Trolls will move in your direction at sight or depending on their smell (fresh meat) power. For example if their smell power is 10, they will be able to move in your direction if they are 10 cells away from you or less. In other situations they will move randomly or rest.

First you need to enter the settings for the game

- View size (rows/columns), choose the values that will best fit your console

- Initial maze size (in terms of rooms, see below for different types of cell)

- Your initial switch power

- The initial number of trolls and their smell power

- And finally maze growth, switch power bonus and additional number of trolls/smell power when you go to next level

You will be able to see only cells that are accessible and in a straight line, and all their neighbour cells.

Different types of cells are

- Corners/Borders/Exit (@/O/X): not accessible except for the exit of course.

- Rooms (+): you will be able to switch neighbour walls/corridors only when you are in a room. In the generated maze rooms are alternating with walls/corridors.

- Walls ('#')

- Corridors (' ')

- Hidden cells ('?')

At each turn, you can take one of the following decisions

- w / n / e / s (move west/north/east/south)

- W / N / E / S (switch west/north/east/south cell)

- r (rest)

- q (quit)
