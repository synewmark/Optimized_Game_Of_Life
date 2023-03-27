package game_of_life;

public class GameOfLifeGPU extends GameOfLifeAbstract {
	static {
//		System.loadLibrary("binary/" + "libwinpthread-1");
		System.loadLibrary("binary/" + "native_gpu");
	}
	public static final int threadcount = 8;
	private final static String name = "GPU Technique";
	private final static String description = "For loop check each adjacent";
	private static byte[] lookuptable;

	private final boolean[][] temp;

	public GameOfLifeGPU(boolean[][] board) {
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
		getNGenerationNative(threadcount, lookuptable, n, temp);
		return temp;
	}

	private native void getNGenerationNative(int threadcount, byte[] lookuptable, int n, boolean[][] array);

	private static byte[] generateLookup() {
		byte[] lookup = new byte[1 << 9];
		short count = 0;
		for (int i = 1; i < lookup.length; i++) {
			int change = i ^ (i - 1);
			for (int j = 0; j < 9; j++) {
				if (j == 4) {
					continue;
				}
				if ((change & (1 << j)) != 0) {
					int delta = ((i & (1 << j)) != 0) ? 1 : -1;
					count += delta;
				}
			}
			byte result = 0;
			if (count == 3 || (count == 2 && (i & (1 << (4))) != 0)) {
				result++;
			}
			lookup[i] = result;
		}
		return lookup;
	}

	public static void main(String[] args) {
		byte[] val = generateLookup();
		for (int i = 0; i < 20; i++) {
			int attempt = (int) (Math.random() * (1 << 9));
			int low = attempt & (0x7);
			int mid = (attempt & (0x7 << 3)) >> 3;
			int high = (attempt & (0x7 << 6)) >> 6;

			System.out.println(String.format("%3s", Integer.toBinaryString(high)).replace(" ", "0"));
			System.out.println(String.format("%3s", Integer.toBinaryString(mid)).replace(" ", "0"));
			System.out.println(String.format("%3s", Integer.toBinaryString(low)).replace(" ", "0"));

			System.out.println();
			System.out.println(String.format("%3s", Integer.toBinaryString(val[attempt])).replace(" ", "0"));

			System.out.println();
			System.out.println();

		}

	}
}