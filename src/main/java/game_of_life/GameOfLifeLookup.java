package game_of_life;

public class GameOfLifeLookup extends GameOfLifeAbstract {
	static {
		System.loadLibrary("native_lookup");
	}
	private final static String name = "Native Technique";
	private final static String description = "For loop check each adjacent";
	private static byte[] lookuptable;

	private final boolean[][] temp;

	public GameOfLifeLookup(boolean[][] board) {
		super(name, description, board);
		this.temp = new boolean[board.length][board[0].length];
		for (int i = 0; i < board.length; i++) {
			temp[i] = board[i].clone();
		}
	}

	@Override
	public boolean[][] getNGeneration(int n) {
		if (lookuptable == null) {
			lookuptable = generateLookup();
		}
		System.out.println(lookuptable[0x808080]);
		getNGenerationNative(lookuptable, n, temp);
		return temp;
	}

	private native void getNGenerationNative(byte[] lookuptable, int n, boolean[][] array);

	private static byte[] generateLookup() {
		byte[] lookup = new byte[1 << 30];
		short[] count = new short[12];
		for (int i = 1; i < 1 << 30; i++) {
			int change = i ^ (i - 1);
			int low = change & 0x0003FF;
			int mid = (change & 0xFFC00) >> 10;
			int high = (change & 0x3FF00000) >> 20;
			for (int j = 0; j < 10; j++) {
				if ((low & (1 << j)) != 0) {
					int delta = ((i & (1 << j)) != 0) ? 1 : -1;
					count[j] += delta;
					count[j + 1] += delta;
					count[j + 2] += delta;
				}
			}
			for (int j = 0; j < 10; j++) {
				if ((mid & (1 << j)) != 0) {
					int delta = ((i & (1 << j + 10)) != 0) ? 1 : -1;
					count[j] += delta;
					count[j + 2] += delta;
				}
			}
			for (int j = 0; j < 10; j++) {
				if ((high & (1 << j)) != 0) {
					int delta = ((i & (1 << j + 20)) != 0) ? 1 : -1;
					count[j] += delta;
					count[j + 1] += delta;
					count[j + 2] += delta;
				}
			}
			byte result = 0;
			for (int j = 7; j >= 0; j--) {
				result <<= 1;
				if (count[j + 2] == 3 || (count[j + 2] == 2 && (i & (1 << (j + 11))) != 0)) {
					result++;
				}
			}
			lookup[i] = result;
		}
		return lookup;
	}

//	public static void main(String[] args) {
//		byte[] val = generateLookup();
//		for (int i = 0; i < 20; i++) {
//			int attempt = (int) (Math.random() * (1 << 30));
//			int low = attempt & 0x03FF;
//			int mid = (attempt & 0xFFC00) >> 10;
//			int high = (attempt & 0x3FF00000) >> 20;
//
//			System.out.println(String.format("%10s", Integer.toBinaryString(high)).replace(" ", "0"));
//			System.out.println(String.format("%10s", Integer.toBinaryString(mid)).replace(" ", "0"));
//			System.out.println(String.format("%10s", Integer.toBinaryString(low)).replace(" ", "0"));
//
//			System.out.println();
//			System.out.println(String.format("%10s", Integer.toBinaryString(val[attempt])).replace(" ", "0"));
//
//			System.out.println();
//			System.out.println();
//
//		}
//
//	}
}