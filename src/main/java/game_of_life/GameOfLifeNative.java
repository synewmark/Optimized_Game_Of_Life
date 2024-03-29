package game_of_life;

public class GameOfLifeNative extends GameOfLifeAbstract {
	static {
		System.loadLibrary("binary/" + "native");
	}
	private final static String name = "Native Technique";
	private final static String description = "For loop check each adjacent";

	private final boolean[][] temp;

	public GameOfLifeNative(boolean[][] board) {
		super(name, description, board);
		this.temp = new boolean[board.length][board[0].length];
		for (int i = 0; i < board.length; i++) {
			temp[i] = board[i].clone();
		}
	}

	@Override
	public boolean[][] getNGeneration(int n) {
		getNGenerationNative(n, temp);
		return temp;
	}

	private native void getNGenerationNative(int n, boolean[][] array);

}