package game_of_life;

import java.util.Arrays;

public class GameOfLifeProactive extends GameOfLifeAbstract {
	private final static String name = "Proactive Technique";
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
//		
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < modifiedArray.length; j++) {
				int ylen0 = modifiedArray[(j - 1 + xlen) % xlen].length;

				int ylen1 = modifiedArray[j].length;
				int ylen2 = modifiedArray[(j + 1) % xlen].length;
				for (int k = 0; k < ylen1; k++) {

					if (modifiedArray[j][k]) {
						buffer[(j + 1) % xlen][(k - 1 + ylen2) % ylen2]++;
						buffer[(j + 1) % xlen][k % ylen2]++;
						buffer[(j + 1) % xlen][(k + 1) % ylen2]++;

						buffer[j][(k + 1) % ylen1]++;

						buffer[j][(k - 1 + ylen1) % ylen1]++;

						buffer[(j - 1 + xlen) % xlen][(k - 1 + ylen0) % ylen0]++;
						buffer[(j - 1 + xlen) % xlen][k % ylen0]++;
						buffer[(j - 1 + xlen) % xlen][(k + 1) % ylen0]++;

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
