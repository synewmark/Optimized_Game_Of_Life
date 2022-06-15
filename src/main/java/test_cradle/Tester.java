package test_cradle;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.Scanner;

import game_of_life.GameOfLifeAbstract;
import game_of_life.GameOfLifeProactive;

public class Tester {
	final char alive;
	final char dead;
	final int genlength;

	final boolean[][] initial;
	final boolean[][] expected;

	Tester(File start, File end) throws FileNotFoundException {
		Scanner r = new Scanner(new FileReader(start));
		int x = -1;
		int y = -1;
		String firstLine = r.nextLine();
		String[] header = firstLine.split(" ");
		x = Integer.parseInt(header[1]);
		y = Integer.parseInt(header[2]);
		System.out.println(x + " " + y);
		genlength = Integer.parseInt(header[0]);
		alive = header[3].charAt(0);
		dead = header[4].charAt(0);
		initial = new boolean[x][y];
		for (int i = 0; i < x; i++) {
			String line = r.nextLine();
			for (int j = 0; j < y; j++) {
				initial[i][j] = line.charAt(j) == alive;
			}
		}
		r.close();
		Scanner s = new Scanner(new FileReader(end));
		expected = new boolean[x][y];
		for (int i = 0; i < x; i++) {
			String line = s.nextLine();
			for (int j = 0; j < y; j++) {
				expected[i][j] = line.charAt(j) == alive;
			}
		}

	}

	long run() {
		GameOfLifeAbstract game = new GameOfLifeProactive(initial);
		long start = System.nanoTime();
		boolean[][] result = game.getNGeneration(genlength);
		long end = System.nanoTime();
		if (!Arrays.deepEquals(result, expected)) {
			try {
				PrintWriter out = new PrintWriter("output.txt");
				out.print(printer(result));
				out.close();
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			System.out.println("Failed");
		}
		return end - start;
	}

	String printer(boolean[][] array) {
		StringBuffer sb = new StringBuffer(array.length * (array[0].length + 1));
		for (boolean[] sub : array) {
			for (boolean bool : sub) {
				sb.append(bool ? alive : dead);
			}
			sb.append('\n');
		}
		return sb.toString();
	}

	public static void main(String... args) throws IOException {
		File simple = new File("C:\\Users\\ahome\\git\\Optimized_Game_Of_Life\\test_cases\\large.txt");
		File simpleexp = new File("C:\\Users\\ahome\\git\\Optimized_Game_Of_Life\\test_cases\\large_expected.txt");
//		String content = Files.readString(simple, StandardCharsets.US_ASCII);
//		String content1 = Files.readString(simpleexp, StandardCharsets.US_ASCII);
//		System.out.println(content);
		Tester test = new Tester(simple, simpleexp);
		System.out.println(test.run());
//		randomOutput();
	}

	public static void randomOutput() {
		try {
			int x = 20000;
			int y = 10000;
			int gen = 1000000;
			PrintWriter out = new PrintWriter("output.txt");
			out.println(x + " " + y + " " + gen + " 0 O");
			for (int i = 0; i < 20000; i++) {
				for (int j = 0; j < 10000; j++) {
					out.print(Math.random() < 0.25 ? '0' : 'O');
				}
				out.println();
				System.out.println("Finished line: " + i);
			}
			out.close();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}
}
