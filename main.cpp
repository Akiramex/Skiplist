#include <iostream>
#include "skiplist.h"
#define FILE_PATH "test.txt"

int main() {
	SkipList<int, std::string> skipList(6);
	skipList.insert_element(1, "A1"); 
	skipList.insert_element(3, "B2"); 
	skipList.insert_element(7, "A3"); 
	skipList.insert_element(8, "C1"); 
	skipList.insert_element(9, "D1"); 
	skipList.insert_element(19, "B1"); 
	skipList.insert_element(19, "C2"); 

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.dump_file();

    // skipList.load_file();

    std::ofstream out;
    out.open(FILE_PATH, std::ofstream::out| std::ofstream::app);
    out << "fuckbitch" << std::endl;

    skipList.search_element(9);
    skipList.search_element(18);


    skipList.display_list();

    skipList.delete_element(3);
    skipList.delete_element(7);

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.display_list();
}