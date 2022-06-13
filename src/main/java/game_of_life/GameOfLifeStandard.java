package game_of_life;

public class GameOfLifeStandard extends GameOfLifeAbstract {
	private final static String name = "Standard Technique";
	private final static String description = "For loop check each adjacent";

	private final boolean[][] buffer;

	public GameOfLifeStandard(boolean[][] board) {
		super(name, description, board);
		this.buffer = new boolean[board.length][board[0].length];
	}

	@Override
	public boolean[][] getNGeneration(int n) {
		boolean[][] modifiedArray = this.getBoard().clone();
		boolean[][] buffer = this.buffer;
		for (int i = 0; i < modifiedArray.length; i++) {
			modifiedArray[i] = this.getBoard()[i].clone();
		}
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < modifiedArray.length; j++) {
				for (int k = 0; k < modifiedArray[j].length; k++) {
					buffer[j][k] = check(j, k, modifiedArray);
				}
			}
			var temp = modifiedArray;
			modifiedArray = buffer;
			buffer = temp;
		}
		return modifiedArray;
	}

	private boolean check(int x, int y, boolean[][] array) {
		int total = 0;
		int xlength = array.length;
		int ylength = array[0].length;
		// top left corner + sides
		if (array[(x - 1 + xlength) % xlength][y]) {
			total++;
		}
		if (array[x][(y - 1 + ylength) % ylength]) {
			total++;
		}
		if (array[(x - 1 + xlength) % xlength][(y - 1 + ylength) % ylength]) {
			total++;
		}

		// bottom right corner + sides
		if (array[(x + 1) % xlength][y]) {
			total++;
		}
		if (array[x][(y + 1) % ylength]) {
			total++;
		}
		if (array[(x + 1) % xlength][(y + 1) % ylength]) {
			total++;
		}

		// top right corner
		if (array[(x + 1) % xlength][(y - 1 + ylength) % ylength]) {
			total++;
		}

		// bottom left corner
		if (array[(x - 1 + xlength) % xlength][(y + 1) % ylength]) {
			total++;
		}

		return (total == 3 || (total == 2 && array[x][y]));
	}

}
