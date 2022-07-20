package game_of_life;

public class GameOfLifeMultithread extends GameOfLifeAbstract {
	static {
		System.loadLibrary("libwinpthread-1");
//		System.load("C:\\Windows\\System32\\");
//		System.load("C:\\Windows\\System32\\");

		System.loadLibrary("native_multithread");
	}
	public static final int threadcount = 8;
	private final static String name = "Native Technique";
	private final static String description = "For loop check each adjacent";
	private static byte[] lookuptable;

	private final boolean[][] temp;

	public GameOfLifeMultithread(boolean[][] board) {
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
//		System.out.println(lookuptable[0x808080]);
		getNGenerationNative(threadcount, lookuptable, n, temp);
		return temp;
	}

	private native void getNGenerationNative(int threadcount, byte[] lookuptable, int n, boolean[][] array);

	private static byte[] generateLookup() {
		byte[] lookup = new byte[1 << 18];
		short[] count = new short[8];
		for (int i = 1; i < 1 << 18; i++) {
			int change = i ^ (i - 1);
			int low = change & 0x3F;
			int mid = (change & 0xFC0) >> 6;
			int high = (change & 0x3F000) >> 12;
			for (int j = 0; j < 6; j++) {
				if ((low & (1 << j)) != 0) {
					int delta = ((i & (1 << j)) != 0) ? 1 : -1;
					count[j] += delta;
					count[j + 1] += delta;
					count[j + 2] += delta;
				}
			}
			for (int j = 0; j < 6; j++) {
				if ((mid & (1 << j)) != 0) {
					int delta = ((i & (1 << j + 6)) != 0) ? 1 : -1;
					count[j] += delta;
					count[j + 2] += delta;
				}
			}
			for (int j = 0; j < 6; j++) {
				if ((high & (1 << j)) != 0) {
					int delta = ((i & (1 << j + 12)) != 0) ? 1 : -1;
					count[j] += delta;
					count[j + 1] += delta;
					count[j + 2] += delta;
				}
			}
			byte result = 0;
			for (int j = 3; j >= 0; j--) {
				result <<= 1;
				if (count[j + 2] == 3 || (count[j + 2] == 2 && (i & (1 << (j + 7))) != 0)) {
					result++;
				}
			}
			lookup[i] = result;
//			if (i == 0xc20) {
//				System.out.println(Arrays.toString(count));
//				System.out.println(result);
//				System.out.println();
//			}
		}
		return lookup;
	}

	public static void main(String[] args) {
		byte[] val = generateLookup();
		for (int i = 0; i < 20; i++) {
			int attempt = (int) (Math.random() * (1 << 18));
			int low = attempt & 0x3F;
			int mid = (attempt & 0xFC0) >> 6;
			int high = (attempt & 0x3F000) >> 12;

			System.out.println(String.format("%6s", Integer.toBinaryString(high)).replace(" ", "0"));
			System.out.println(String.format("%6s", Integer.toBinaryString(mid)).replace(" ", "0"));
			System.out.println(String.format("%6s", Integer.toBinaryString(low)).replace(" ", "0"));

			System.out.println();
			System.out.println(String.format("%6s", Integer.toBinaryString(val[attempt])).replace(" ", "0"));

			System.out.println();
			System.out.println();

		}
		System.out.println(val[0x20820]);

	}
}