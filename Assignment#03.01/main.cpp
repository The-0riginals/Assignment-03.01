#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
using namespace std;

#pragma pack(1)
struct PartitionTable {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    unsigned long start_sector;
    unsigned long length_sectors;
};

struct Fat16BootSector {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short total_sectors_short;
    unsigned char media_descriptor;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short number_of_heads;
    unsigned long hidden_sectors;
    unsigned long total_sectors_long;
    unsigned char drive_number;
    unsigned char current_head;
    unsigned char boot_signature;
    unsigned long volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
};

class Fat16 {
public:
    void bootInfo(string link) {
        ifstream in(link, ios::binary);
        int i;
        PartitionTable pt[4];
        Fat16BootSector bs;

        in.seekg(0x1BE); // go to partition table start
        in.read(reinterpret_cast<char*>(pt), sizeof(PartitionTable) * 4); // read all four entries

        //search for a FAT16 partition
        for (i = 0; i < 4; i++) {
            if (pt[i].partition_type == 4 || pt[i].partition_type == 6 ||
                pt[i].partition_type == 14) {
                cout << "FAT16 filesystem found from partition " << i << endl;
                break;
            }
        }
        //if no FAT16 partition was found, print an error message and exit
        if (i == 4) {
            cout << "No FAT16 filesystem found, exiting..." << endl;
            return;
        }

        //print the partition table entry for the FAT16 partition
        //the boot sector of the FAT16 partition is located at the logical sector stored in the start_sector field of the partition table entry
        cout << "\nSector logic start: " << dec << pt[i].start_sector << endl;    

        //move to the boot sector (the first logical sector of the partition) and read the boot sector:
        in.seekg(512 * pt[i].start_sector); // Move to boot sector
        in.read(reinterpret_cast<char*>(&bs), sizeof(Fat16BootSector)); // Read boot sector
        
        // Print the boot sector in hexadecimal
        cout << "\nBoot sector: " << endl;
        for (int j = 0; j < 512; ++j) {
            cout << hex << setw(2) << setfill('0') << int(reinterpret_cast<unsigned char*>(&bs)[j]) << " ";
            if ((j + 1) % 16 == 0) {
                cout << endl;
            }
        }
        /*setw(2) is used to ensure that each hexadecimal number printed is at least 2 characters wide.
        If a number is less than 2 characters, it will be padded with leading zeros using setfill('0').
        For example, if a number is A, it will be printed as 0A.
        */

        // Print the boot sector information:
        cout << "\nInformation about the boot sector: " << endl;
		cout << "  + Jump code: " << hex << setw(2) << setfill('0') << int(bs.jmp[0]) << ":"
			<< hex << setw(2) << setfill('0') << int(bs.jmp[1]) << ":"
			<< hex << setw(2) << setfill('0') << int(bs.jmp[2]) << endl;
		cout << "  + OEM code: [" << bs.oem << "]" << endl;
		cout << "  + sector_size: " << dec << bs.sector_size << endl;
		cout << "  + sectors_per_cluster: " << int(bs.sectors_per_cluster) << endl;
		cout << "  + reserved_sectors: " << bs.reserved_sectors << endl;
		cout << "  + number_of_fats: " << int(bs.number_of_fats) << endl;
		cout << "  + root_dir_entries: " << bs.root_dir_entries << endl;
		cout << "  + total_sectors_short: " << bs.total_sectors_short << endl;
		cout << "  + media_descriptor: 0x" << hex << int(bs.media_descriptor) << endl;
		cout << "  + fat_size_sectors: " << dec << bs.fat_size_sectors << endl;
		cout << "  + sectors_per_track: " << dec << bs.sectors_per_track << endl;
		cout << "  + number_of_heads: " << dec << bs.number_of_heads << endl;
		cout << "  + hidden_sectors: " << dec << bs.hidden_sectors << endl;
		cout << "  + total_sectors_long: "<< dec << bs.total_sectors_long << endl;
		cout << "  + drive_number: 0x" << hex << int(bs.drive_number) << endl;
		cout << "  + current_head: 0x" << hex << int(bs.current_head) << endl;
		cout << "  + boot_signature: 0x" << hex << int(bs.boot_signature) << endl;
		cout << "  + volume_id: 0x" << hex << bs.volume_id << endl;
		cout << "  + Volume label: [" << bs.volume_label << "]" << endl;
		cout << "  + Filesystem type: [" << bs.fs_type << "]" << endl;
		cout << "  + Boot sector signature: 0x" << hex << bs.boot_sector_signature << endl;   
    }
};

//class image file
class ImageFile {
    private:
        streamsize g_imageSize = 1024 * 1024; // Default image size (1 MB)
        std::streamsize g_currentPosition = 0; // Current position in the file
public:
    void createImageFile(const std::string& fileName) {
        // Open the file in binary mode
        std::ofstream outFile(fileName, std::ios::binary);

        if (outFile.is_open()) {
            // Set the file size
            outFile.seekp(g_imageSize - 1);
            outFile.put(0);

            // Close the file
            outFile.close();

            std::cout << "Image file '" << fileName << "' created successfully." << std::endl;
        }
        else {
            std::cerr << "Error: Unable to create image file '" << fileName << "'." << std::endl;
        }
    }

    // Load the application state from the file
    void loadApplicationState() {
        std::ifstream stateFile("app_state.dat", std::ios::binary);

        if (stateFile.is_open()) {
            stateFile.read(reinterpret_cast<char*>(&g_imageSize), sizeof(std::streamsize));
            stateFile.read(reinterpret_cast<char*>(&g_currentPosition), sizeof(std::streamsize));
            stateFile.close();
        }
    }

    // Save the application state to the file
    void saveApplicationState() {
        std::ofstream stateFile("app_state.dat", std::ios::binary);

        if (stateFile.is_open()) {
            stateFile.write(reinterpret_cast<const char*>(&g_imageSize), sizeof(std::streamsize));
            stateFile.write(reinterpret_cast<const char*>(&g_currentPosition), sizeof(std::streamsize));
            stateFile.close();
        }
    }

    // Getters and setters
    std::streamsize getCurrentPosition() {
        return g_currentPosition;
    }

    // Set the current position in the file
    void setCurrentPosition(std::streamsize position) {
        g_currentPosition = position;
    }

    // get the image size
    std::streamsize getImageSize() {
		return g_imageSize;
	}
};

int main() {
    ImageFile imageFile;
    imageFile.loadApplicationState();

    // ----------------------
    while (true) {
        short int choose;
        cout << "\n0 to Exit | 1 to read Bootsector | 2 to Create img file : ";
        cin >> choose;

        if (choose == 0) break;
		else if (choose == 1) {
            string link;
            cout << "Enter the link: ";
            cin >> link;
            Fat16 fat16;
            fat16.bootInfo(link);
        }
        else if (choose == 2) {
            string fileName;
            cout << "Enter the file name: ";
            cin >> fileName;

            std::streamsize lastPosition = imageFile.getCurrentPosition();
            std::streamsize imageSize = imageFile.getImageSize();

            std::ifstream checkFile(fileName, std::ios::binary);

            if (checkFile.is_open()) {
                // If the image file already exists, check if it's complete
                checkFile.seekg(0, std::ios::end);
                std::streamsize fileSize = checkFile.tellg();
                checkFile.close();

                if (fileSize == imageSize) {
                    cout << "Image file '" << fileName << "' is already complete." << endl;
                }
                else {
                    cout << "Resuming image file creation..." << endl;
                    std::ofstream outFile(fileName, std::ios::binary | std::ios::app);
                    outFile.seekp(lastPosition);

                    // Simulate writing some data (10 bytes with "0")
                    outFile.write(0, 10);

                    outFile.close();

                    // Update current position
                    imageFile.setCurrentPosition(lastPosition + 10);

                    cout << "Resumed writing from position " << lastPosition << "." << endl;
                }
            }
            else {
                // If the image file doesn't exist, create it
                imageFile.createImageFile(fileName);
            }

            // Save application state for future runs
            imageFile.saveApplicationState();
        }
        else {
            cout << "Invalid input!" << endl;
        }
    }

    return 0;
}