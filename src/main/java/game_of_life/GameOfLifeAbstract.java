package game_of_life;

public abstract class GameOfLifeAbstract {
	final boolean[][] board;

	private final String name;
	private final String description;

	GameOfLifeAbstract(String name, String description, boolean[][] board) {
		this.name = name;
		this.description = description;
		this.board = board;
	}

	public String getName() {
		return name;
	}

	public String getDescription() {
		return description;
	}

	boolean[][] getBoard() {
		return board;
	}

	public abstract boolean[][] getNGeneration(int n);
}
