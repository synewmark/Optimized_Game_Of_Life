package game_of_life;

public class GameOfLifeMultithread extends GameOfLifeAbstract {
	static {
		System.loadLibrary("binary/" + "native_multithread");
	}
	/*-
	 * Indexing will look like this (P is padding, C is center:					
	 * 		PPPPPP
	 * 		PCCCCP
	 * 		PPPPPP
	 * 	However, when indexing these values will be concatenated giving the following 
	 * 	bit string:
	 * 		PPPPPPPCCCCPPPPPPP
	 */

	// Size of the value returned by a lookup
	public static final int LOOKUP_LEN = 4;
	// The row length should be the lookup length +2 for padding on either side
	public static final int ROW_LEN = LOOKUP_LEN + 2;
	// Additionally, pad the top and bottom
	public static final int TOTAL_LOOKUP_SIZE = ROW_LEN * 3;

	public static final int threadcount = 8;
	private final static String name = "Multithreaded Technique";
	private final static String description = "Uses a lookuptable and dirty bits with multithreading";
	private static byte[] lookuptable;

	public GameOfLifeMultithread(boolean[][] board) {
		super(name, description, board);
	}

	@Override
	public boolean[][] getNGeneration(int n) {
		if (lookuptable == null) {
			lookuptable = generateLookup();
		}
		getNGenerationNative(threadcount, lookuptable, n, this.getBoard());
		return this.getBoard();
	}

	private native void getNGenerationNative(int threadcount, byte[] lookuptable, int n, boolean[][] array);

	private static byte[] generateLookup() {
		byte[] lookup = new byte[1 << (TOTAL_LOOKUP_SIZE)];
		// Adjacency count for center cells, give +2 padding, 1 for each side, not
		// because we need it, but to simplify indexing the array and avoid checking for
		// under/over-flow
		byte[] count = new byte[ROW_LEN + 2];
		// Skip 0 because this doesn't work through underflow
		// An all 0 bitfield should return 0 either way, so no special logic is needed
		for (int i = 1; i < lookup.length; i++) {
			// Track changes between the current bit-field and the previous one
			int change = i ^ (i - 1);
			for (int j = 0; j < TOTAL_LOOKUP_SIZE; j++) {
				// Check if a given position was changed between this one and the last
				if ((change & (1 << j)) != 0) {
					// If it was changed, add 1 to all adjacent if it was "born" or -1 if it "died"
					int delta = ((i & (1 << j)) != 0) ? 1 : -1;
					count[(j) % ROW_LEN] += delta;
					// In GOL, you don't count the cell itself in the live count, so if we're in the
					// center row, don't increment the current cell's count, only the adjacent cells
					if (j < ROW_LEN || j > TOTAL_LOOKUP_SIZE - ROW_LEN) {
						count[(j + 1) % ROW_LEN] += delta;
					}
					count[(j + 2) % ROW_LEN] += delta;
				}
			}
			byte result = 0;
			for (int j = LOOKUP_LEN; j > 0; j--) {
				result <<= 1;
				// Because we gave count array padding, we need to +1 to get the actual value
				if (count[j + 1] == 3 || (count[j + 1] == 2 && (i & (1 << (j + ROW_LEN))) != 0)) {
					result++;
				}
			}
			lookup[i] = result;

//			if (i == lookup.length - 1) {
//				System.out.println(String.format("%x", i));
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
			System.out
					.println("0" + String.format("%4s", Integer.toBinaryString(val[attempt])).replace(" ", "0") + "0");

			System.out.println();
			System.out.println();

		}
		System.out.println(val[0x20820]);

	}
}