package game_of_life;

public class GameOfLifePacked extends GameOfLifeAbstract {
	private final static String name = "Standard Technique";
	private final static String description = "For loop check each adjacent";

	private final byte[][] temp;
	private final boolean[][] board;

	private final byte[][][][][] lookupTable;

	public GameOfLifePacked(boolean[][] board) {
		super(name, description, board);
		this.temp = new byte[board.length][board[0].length / 8];
		this.board = board;
		this.lookupTable = generateLookUpTable();
	}

	static int countAdjacent(byte[] array, int x, int y) {
		int count = 0;
		if (x > 0 && ((array[x - 1] >> (y)) & 1) != 0) {
//			System.out.println("x-1");
			count++;
		}
		if (y > 0 && ((array[x] >> (y - 1)) & 1) != 0) {
//			System.out.println("y-1");
			count++;
		}
		if (x > 0 && y > 0 && ((array[x - 1] >> (y - 1)) & 1) != 0) {
//			System.out.println("x-1 y-1");
			count++;
		}

		if (x < 3 && ((array[x + 1] >> y) & 1) != 0) {
//			System.out.println("x+1");
			count++;
		}
		if (((array[x] >> (y + 1)) & 1) != 0) {
//			System.out.println("y+1");
			count++;
		}
		if (x < 3 && ((array[x + 1] >> (y + 1)) & 1) != 0) {
//			System.out.println("x+1 y+1");
			count++;
		}

		if (x < 3 && y > 0 && ((array[x + 1] >> y - 1) & 1) != 0) {
//			System.out.println("x+1 y-1");
			count++;
		}
		if (x > 0 && ((array[x - 1] >> (y + 1)) & 1) != 0) {
//			System.out.println("x-1 y+1");
			count++;
		}
		return count;
	}

	static byte[] makeTable(byte[] array) {
		byte[] newArray = new byte[4];
		for (byte i = 0; i < 4; i++) {
			byte c = 0;
			for (byte j = 0; j < 4; j++) {
//				System.out.println(i + " " + j);
				int adj = countAdjacent(array, i, j);
//				System.out.println(adj);
//				System.out.println(((array[i] >> j) & 1) != 0);
				if (adj == 3 || (((array[i] >> j) & 1) != 0 && adj == 2)) {
					System.out.println("!!");
					c |= (1 << j);
				}
			}
			newArray[i] = c;
		}
		return newArray;
	}

	static byte[][][][][] generateLookUpTable() {
		byte[][][][][] table = new byte[4][4][4][4][];
		for (byte i = 0; i < 4; i++) {
			for (byte j = 0; j < 4; j++) {
				for (byte k = 0; k < 4; k++) {
					for (byte l = 0; l < 4; l++) {
						byte[] array = { i, j, k, l };
						table[i][j][k][l] = makeTable(array);
					}
				}
			}
		}
		return table;
	}

	byte[][] pack(boolean[][] array) {
		byte[][] packed = new byte[array.length][array.length / 8];
		for (int i = 0; i < array.length; i++) {
			for (int j = 0; j < array[0].length; j += 4) {
				byte n = 0;
				for (int k = 0; k < 4; k++) {
					if (array[i][j + k]) {
						n |= (1 << k);
					}
				}
				packed[i][j] = n;
			}
		}
		return packed;
	}

	void combineHoro(byte[] store, byte[] a, byte[] b) {
		for (int i = 0; i < 4; i++) {
			store[i] = (byte) ((a[i] << 2) & (b[i] >>> 2) & 0x0F);
		}
	}

	void combineVert(byte[] store, byte[] a, byte[] b) {
		store[0] = a[2];
		store[1] = a[3];
		store[2] = b[0];
		store[3] = b[1];
	}

	@Override
	public boolean[][] getNGeneration(int n) {
		byte[][] packed = pack(board);
		byte[] buf = new byte[4];
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < packed.length; j++) {
				for (int k = 0; k < packed[j].length; k++) {

				}
			}
		}
		return null;
	}

	public static void main(String... strings) {
		byte[] n = { 0, Byte.MAX_VALUE & 0xF, Byte.MAX_VALUE & 0xF, 0 };
		byte x = 3;
		System.out.println(countAdjacent(n, 0, 3));
//		System.out.println((n[1] >> 0) & 1);
		byte[] array = makeTable(n);
		for (int i = 0; i < 4; i++) {
			System.out.println(Integer.toBinaryString(array[i]));
		}
	}
}