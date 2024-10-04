/*=============================================================================
| Assignment: pa02 - Calculating an 8, 16, or 32 bit
| checksum on an ASCII input file
|
| Author: Patrik De Quevedo
| Language: c
|
| To Compile: gcc -o pa02 pa02.c
| 
| To Execute: ./pa02 inputFile.txt 8
| 
| where inputFile.txt is an ASCII input file
| and the number 8 could also be 16 or 32
| which are the valid checksum sizes, all
| other values are rejected with an error message
| and program termination
|
| Note: All input files are simple 8 bit ASCII input
|
| Class: CIS3360 - Security in Computing - Spring 2024
| Instructor: McAlpin
| Due Date: March 31, 2024
|
+=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//uint#_t variables are to get vars for a specified bit size. Ex: uint32_t = bit of size 32 (takes up exactly 32 bits of space)
//size_t is chosen bc it can store the maximum size of a theoretically possible array or an object. It is the type returned by "sizeof"


// Function to calculate padding bytes needed based on data length and checkSum size
size_t calculatePadding(size_t len, int checkSumBit) {

    if (checkSumBit == 32) {
        return (4 - (len % 4)) % 4; // For 32-bit checksum, calculate the number of padding bytes needed to make the data length a multiple of 4
    }

    else if (checkSumBit == 16) {
        return len % 2; // For 16-bit checksum, need 1 byte padding if data length is odd
    }

    return 0; // No padding needed for 8-bit checksum
}

// Function to print padding 'X' characters
void printPadding(size_t padding) {
    for (int i = 0; i < padding; i++) {
        printf("X");
    }
}

// Calculates checkSum 8-bits
uint32_t calculateChecksum8(const uint8_t *data, size_t len, size_t *padding) {

    uint64_t checkSum = 0;
    *padding = 0; // No padding needed for 8-bit checksums
    
    
    for (size_t i = 0; i < len; ++i) {
        checkSum = (data[i] + checkSum) & 0xFF; // Makes the sum stay within 8 bits
    }

    return (uint32_t)checkSum;
}

// Calculates checkSum 16-bits 
uint32_t calculateChecksum16(const uint8_t *data, size_t len, size_t *padding) {

    uint32_t checkSum = 0;
    uint16_t sum = 0;
    uint16_t overflow = 0;

    *padding = calculatePadding(len, 16); // Calculate padding bytes needed

    // Forms 16-bit words
    for (size_t i = 0; i + 1 < len; i += 2) {
        sum = data[1+i] | (data[i] << 8);
        checkSum += sum;
        checkSum &= 0xFFFF; // Makes checksum stay within 16 bits
    }

    // odd number of bytes -> pad the last byte with 'X': (0x58)
    if (len % 2) {
        sum = (data[len - 1] << 8) | 0x58;
        checkSum += sum;
    }

    overflow = checkSum >> 16;
    checkSum = (overflow+checkSum) & 0xFFFF; // Final checksum result

    return checkSum;
}

// Calculates checksum 32-bits
uint32_t calculateChecksum32(const uint8_t *data, size_t len, size_t *padding) {

    uint64_t checkSum = 0;

    *padding = calculatePadding(len, 32); // Calculate padding bytes needed

    for (size_t i = 0; i < len; ++i) {

        //  Adds bytes into checksum, shifts byte into position
        checkSum += ((uint32_t)data[i]) << (24 - (i % 4) * 8);

        // Every 4 bytes deals with overflow
        if ((i == len - 1) || (i % 4 == 3)) {

            while (checkSum >> 32) {
                checkSum = (checkSum >> 32) + (checkSum & 0xFFFFFFFF);
            }
        }
    }

    // Padding if len isnt a multiple of 4
    if (len % 4 != 0) {

        size_t padLength = 4 - (len % 4);

        for (size_t pad = 0; pad < padLength; ++pad) {
            checkSum += (uint32_t)'X' << ((padLength - pad - 1) * 8);
        }
        
        while (checkSum >> 32) {
            checkSum = (checkSum >> 32) + (checkSum & 0xFFFFFFFF);
        }

    }

    return (uint32_t)checkSum; 
}

// Function to calculate checksum based on specified bit size (8,16,32)
uint32_t calculateChecksum(const uint8_t *data, size_t len, size_t *padding, int checkSumBit) {

    if (checkSumBit == 8) {
        return calculateChecksum8(data, len, padding);
    } 

    else if (checkSumBit == 16) {
        return calculateChecksum16(data, len, padding);
    } 

    else if (checkSumBit == 32) {
        return calculateChecksum32(data, len, padding);
    }

    return 0; 
}

int main(int argc, char *argv[]) {

    // If you don't put a # after file name
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file name> <checksum size>: Integer expected after File name\n", argv[0]);

        return 1;
    }

    
    int checkSumBit = atoi(argv[2]);    //  str to int

    // Makes sure bit has proper size (8,16,32)
    if (checkSumBit != 8 && checkSumBit != 16 && checkSumBit != 32) {
        fprintf(stderr, "Checksum bit sizes have to be 8, 16, or 32\n"); //  sent to stderr due to instructions

        return 1;
    }

    // Opens file
    const char *fileName = argv[1];

    FILE *fileIN = fopen(fileName, "rb");

    if (!fileIN) {

        perror("File Opening Error");

        return 1;
    }

    // Determine file size for memory & allocates memory for it
    fseek(fileIN, 0, SEEK_END);
    size_t fileSize = ftell(fileIN);
    fseek(fileIN, 0, SEEK_SET);

    
    uint8_t *buffer = (uint8_t *)malloc(fileSize);

    if (!buffer) {

        perror("Memory could not be allocated");
        fclose(fileIN);

        return 1;
    }
    //Determine... End

    fread(buffer, 1, fileSize, fileIN);
    printf("\n");

    // Echo the file onto the screen
    for (size_t i = 0; i < fileSize; ++i) {
        printf("%c", buffer[i]);
    }

    //  Calculations
    size_t padding = 0;
    uint32_t checksum = calculateChecksum(buffer, fileSize, &padding, checkSumBit);

    // Print padding if used
    printPadding(padding);
    printf("\n");

    // Print the calculated checksum
    printf("%2d bit checksum is %8lx for all %4zu chars\n", checkSumBit, (unsigned long)checksum, fileSize);

    fclose(fileIN);
    free(buffer);

    return 0;
}

/*=============================================================================
| I [Patrik De Quevedo] ([5155860]) affirm that this program is
| entirely my own work and that I have neither developed my code together with
| any another person, nor copied any code from any other person, nor permitted
| my code to be copied or otherwise used by any other person, nor have I
| copied, modified, or otherwise used programs created by others. I acknowledge
| that any violation of the above terms will be treated as academic dishonesty.
+============================================================================*/