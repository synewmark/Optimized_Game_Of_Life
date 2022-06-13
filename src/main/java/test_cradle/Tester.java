package test_cradle;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;

import game_of_life.GameOfLifeAbstract;
import game_of_life.GameOfLifeProactive;

public class Tester {
	final char alive;
	final char dead;
	final int genlength;

	Tester(String start, String end) {
		String[] splitArray = start.split("\\r?\\n");
		int x = -1;
		int y = -1;
		String[] header = splitArray[0].split(" ");
		x = Integer.parseInt(header[1]);
		y = Integer.parseInt(header[2]);
		genlength = Integer.parseInt(header[0]);
		alive = header[3].charAt(0);
		dead = header[4].charAt(0);
		boolean[][] array = new boolean[x][y];
		for (int i = 0; i < x; i++) {
			for (int j = 0; j < y; j++) {
				array[i][j] = splitArray[i + 1].charAt(j) == alive;
			}
		}
		String[] expectedValues = end.split("\\r?\\n");
		boolean[][] expected = new boolean[x][y];
		for (int i = 0; i < x; i++) {
			for (int j = 0; j < y; j++) {
				expected[i][j] = expectedValues[i].charAt(j) == alive;
			}
		}
		GameOfLifeAbstract game = new GameOfLifeProactive(array);
		boolean[][] result = game.getNGeneration(genlength);
//		System.out.println(printer(result));
		System.out.println(Arrays.deepEquals(expected, result));

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
		Path simple = new File("C:\\Users\\ahome\\git\\Optimized_Game_Of_Life\\test_cases\\simple.txt").toPath();
		Path simpleexp = new File("C:\\Users\\ahome\\git\\Optimized_Game_Of_Life\\test_cases\\simple_expected.txt")
				.toPath();
		String content = Files.readString(simple, StandardCharsets.US_ASCII);
		String content1 = Files.readString(simpleexp, StandardCharsets.US_ASCII);
//		System.out.println(content);
		new Tester(content, content1);
	}
}
