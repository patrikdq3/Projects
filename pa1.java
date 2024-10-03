/*============================================================================
| Assignment: pa01 - Encrypting a plaintext file using the Hill cipher
|
| Author: Patrik De Quevedo
| Language: Java
|
| To Compile: javac pa01.java
| 
| To Execute: java -> java pa01 kX.txt pX.txt
| 
| where kX.txt is the keytext file and pX.txt is plaintext file
|
| Note:
| All input files are simple 8 bit ASCII input
| All execute commands above have been tested on Eustis
|
| Class: CIS3360 - Security in Computing - Spring 2024
| Instructor: McAlpin
| Due Date: March 6, 2024
+===========================================================================*/
import java.io.*;
import java.util.*;

public class pa01 {
	
    public static void main(String[] args) throws IOException {
    	
        // Read key matrix from file -> into array
        int[][] Matrixkey = readKeyMatrixFromFile(args[0]);

        // Print key matrix
        System.out.println("Key matrix:");
        printMatrix(Matrixkey);

        // Read plaintext from file and add padding -> into an array
        int n = Matrixkey.length;
        char[] PlainText = readPlaintextFromFile(args[1], n);

        // Print plaintext
        System.out.println("Plaintext:");
        printText(PlainText);

        // Generate ciphertext
        char[] encryptedText = createCiphertext(Matrixkey, PlainText, n);
        
        // Print ciphertext
        System.out.println("");
        System.out.println("CipherText:");
        printText(encryptedText);
    }
    
 // Method to read key matrix from file
    public static int[][] readKeyMatrixFromFile(String filename) throws IOException {
        File file = new File(filename);
        Scanner sc = new Scanner(file);

        int n = sc.nextInt();
        int[][] matrix = new int[n][n];

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                matrix[i][j] = sc.nextInt();
            }
        }

        sc.close();
        return matrix;
    }
    
 // Method to print the key matrix
    public static void printMatrix(int[][] matrix) {
        for (int i = 0; i < matrix.length; i++) {
            for (int j = 0; j < matrix[i].length; j++) {
            	int num = matrix[i][j];
                System.out.print("  "+num);
            }
            System.out.println("");	//Makes every "row" of matrix
        }
        System.out.println("");
    }
    
 // Method to read plaintext from file and add padding
    public static char[] readPlaintextFromFile(String filename, int n) throws IOException {
    	
        File file = new File(filename);
        FileReader reader = new FileReader(file);
        BufferedReader buffer = new BufferedReader(reader);

        char[] plainText = new char[10000];
        int lineLength = 0;
        int possibleChar = 0;
        
        //reads until eof (possibleChar == -1 if eof)
        while((possibleChar = buffer.read()) != -1) {
            char confirmedcharacter = (char) possibleChar;
            
            // Process each character of plaintext
            if(Character.isLetter(possibleChar)) {
            	plainText[lineLength] = confirmedcharacter;
                lineLength++;
            }
        }

        buffer.close();

        // Add padding to ensure plaintext length is divisible by n
        while((lineLength % n) != 0) {
        	plainText[lineLength] = 'x';
            lineLength++;
        }
        
        return plainText;
    }

 // Method to print text (plaintext or ciphertext) to the screen
    public static void printText(char[] text) {
    	int m=0;
    	while(Character.isLetter(text[m])) {	//checks for letter before printing
            System.out.print(Character.toLowerCase(text[m]));	
            if((m+1)%80==0) System.out.println("");		//80 chars per line
            m++;							
        }
        System.out.println("");
    }
    
// Method to create ciphertext from plaintext and key matrix
    public static char[] createCiphertext(int[][] Matrix, char[] PlainText, int n) {
    	
    	//Gets # of used places in array to use linelength
    	int p=0;
    	while(Character.isLetter(PlainText[p])) {
    	p++;	
    	}
    	
        int LineLength = p;	
       
        char[] EncryptedText = new char[10000];
        int ETidx = 0;

        // Process plaintext in blocks of "n" and encrypt
        for (int i = 0; i < LineLength; i += n) {	//Processes each block of side "n"
            int[] temp = new int[n];
            
            // Convert plaintext block to numerical values
            for (int j = 0; j < n; j++) {	//accesess elements in block
                temp[j] = Character.toLowerCase(PlainText[i + j]) - 'a';
            }

            // Encrypt the block "temp"
            int[] NUMencryptedBlock = encryptData(Matrix, temp);

            // Convert encrypted block back to characters
            for (int k = 0; k < n; k++) {
                EncryptedText[ETidx] = (char) (NUMencryptedBlock[k] + 'a');
                ETidx++;
            }
        }
        return EncryptedText;
    }
    
// Algorithm to encrypt plaintext using key matrix
    public static int[] encryptData(int[][] MatrixKey, int[] plainTextBlock) {
        int[] encryptedData = new int[MatrixKey.length];
        
        // Encrypts plaintext block using the key matrix
        for (int i = 0; i < MatrixKey.length; i++) {
            for (int j = 0; j < MatrixKey[i].length; j++){
                encryptedData[i] += plainTextBlock[j] * MatrixKey[i][j];
            }
            encryptedData[i] %= 26;	//To stay in alphabet
        }
        return encryptedData;
    }

}//class end


/*=============================================================================
| I [Patrik De Quevedo] ([5155860]) affirm that this program is
| entirely my own work and that I have neither developed my code together with
| any another person, nor copied any code from any other person, nor permitted
| my code to be copied or otherwise used by any other person, nor have I
| copied, modified, or otherwise used programs created by others. I acknowledge
| that any violation of the above terms will be treated as academic dishonesty.
+=============================================================================*/