package game_of_life;

import java.util.Arrays;

public class GameOfLifeProactive extends GameOfLifeAbstract {
	private final static String name = "Standard Technique";
	private final static String description = "For loop check each adjacent";

	private final byte[][] buffer;

	public GameOfLifeProactive(boolean[][] board) {
		super(name, description, board);
		this.buffer = new byte[board.length][board[0].length];
	}

	@Override
	public boolean[][] getNGeneration(int n) {
		boolean[][] modifiedArray = this.getBoard().clone();
		byte[][] buffer = this.buffer;
		for (int i = 0; i < modifiedArray.length; i++) {
			modifiedArray[i] = this.getBoard()[i].clone();
		}
		int xlen = modifiedArray.length;
		int ylen = modifiedArray[0].length;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < modifiedArray.length; j++) {

				for (int k = 0; k < modifiedArray[j].length; k++) {
					if (modifiedArray[j][k]) {
						buffer[(j + 1) % xlen][k]++;
						buffer[j][(k + 1) % ylen]++;
						buffer[(j + 1) % xlen][(k + 1) % ylen]++;

						buffer[(j - 1 + xlen) % xlen][k]++;
						buffer[j][(k - 1 + ylen) % ylen]++;
						buffer[(j - 1 + xlen) % xlen][(k - 1 + ylen) % ylen]++;

						buffer[(j - 1 + xlen) % xlen][(k + 1) % ylen]++;

						buffer[(j + 1) % xlen][(k - 1 + ylen) % ylen]++;
					}
				}
			}

			for (int j = 0; j < modifiedArray.length; j++) {
				for (int k = 0; k < modifiedArray[j].length; k++) {
					modifiedArray[j][k] = (buffer[j][k] == 3 || (buffer[j][k] == 2 && modifiedArray[j][k]));
				}
				Arrays.fill(buffer[j], (byte) 0);
			}

		}
		return modifiedArray;
	}

}
