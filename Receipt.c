#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC
#include <stdio.h>
#include <stdlib.h>

struct ItemStruct{
	int ItemCode;
	int NumItems;
	float ItemPrice;
	float TotSales;
	char ItemName[17];
	struct ItemStruct* nextItem;
};

struct Customer{
	int customer_id;
	struct ItemStruct* cart;
	struct Customer* nextCustomer;
};

struct Category{
	int category_code;
	char category_name[25];
	struct ItemStruct* category_items;
};

void loadCategories();
void loadProducts();
void addProduct(int category_index, int code, char name[], float price);
void swapProducts(struct ItemStruct* one, struct ItemStruct* two);
void sortList(struct ItemStruct* head);
void printCustomerReceipts();
void printCategoryInventory();

//Total number fo customers
int totalCustomers = 0;
//total number of different categories 
int totalCategories = 0;
//Static memory, array of struct to store Categories data.
struct Category categories[10];
//Dynamics list of all customers
struct Customer* customers;

int main() {

	//Open a file to read categories in read only mode.
	//Check CategoryName.dat  is available in project dir.
	
	loadCategories();
	loadProducts();
	int response ;
	do {
		printf("1. Process customer list : \n2. Generate reports.\n");
		int m =scanf("%d", &response);
		if (response == 1) {
			char filename[100];
			printf("Enter file name to load customer's data:");
			int m = scanf("%s", filename);
			processCustomer(filename);
		}
	}
	while (response != 2);

	printCustomerReceipts();
	printCategoryInventory();
	dailySummaryReport();
	return 0;
}

void loadCategories() {
	FILE* categoryFile;
	fopen_s(&categoryFile, "CategoryName.dat", "r");

	if (!categoryFile) {
		printf("Could not load CategoryName.dat file! \n Leaving the program....\n");
		exit(-1);
	}

	while (!feof(categoryFile)) {
		static int i = 0;
		int j = fscanf_s(categoryFile, "%d", &(categories[i].category_code));
		fscanf_s(categoryFile, "%*[ \t]%[^\n]", categories[i].category_name, 20);
		categories[i].category_items = NULL;
		totalCategories = ++i;
	}
}

void loadProducts() {
	FILE* productFile;
	fopen_s(&productFile, "CodeNamePrice.dat", "r");

	if (!productFile) {
		printf("Could not load CodeNamePrice.dat file! \n Leaving the program....\n");
		exit(-1);
	}

	while (!feof(productFile) ) {
		static int i = 0;
		int code;
		char name[17];
		float price;
		fscanf_s(productFile, "%d", &code);
		char ch = fgetc(productFile);
		while ((ch == 32) || (ch == 9 )) {
			ch = fgetc(productFile);
			continue;
		}
		ungetc(ch, productFile);
		fgets(name, 16, productFile);
		name[16] = '\0';
		for (int m = 16; m >= 0; m--) {
			char c = name[m];
			if ((c == 32) || (c == 9))
				name[m] = '\0';
			if (isalpha(c)) break;
		}
		while ((ch = fgetc(productFile)) == " " || (ch = fgetc(productFile)) == "\t")
			continue;
		fscanf_s(productFile, "%f", &price);

		int category_index = ( (code/100) - 1);
		addProduct(category_index, code, name, price);
	}
}

void addProduct(int category_index, int code, char name[], float price) {
	struct ItemStruct* head = categories[category_index].category_items;
	
	if (head == NULL) {
	//	printf("  Adding product in head. Index: |%d| \n", category_index);
		head = (struct ItemStruct*)malloc(1 * sizeof(struct ItemStruct));
		if (head == NULL) return;
		head->ItemCode = code;
		strcpy(head->ItemName, name);
		head->ItemPrice = price;
		head->NumItems = 0;
		head->TotSales = 0;
		head->nextItem = categories[category_index].category_items;
		categories[category_index].category_items = head;
		
	}
	else {
		while (head->nextItem != NULL)
			head = head->nextItem;
//		printf("  Adding product in end. Index: |%d| \n", category_index);
		struct ItemStruct* newProduct = (struct ItemStruct*)malloc(1 * sizeof(struct ItemStruct));
		if (newProduct == NULL) return;
		newProduct->ItemCode = code;
		strcpy(newProduct->ItemName,name);
		newProduct->ItemPrice = price;
		newProduct->NumItems = 0;
		newProduct->TotSales = 0;
		newProduct->nextItem = NULL;

		head->nextItem = newProduct;
	}
}

void processCustomer(char* filename) {
	FILE* customerFile;
	fopen_s(&customerFile, filename, "r");

	if (!customerFile) {
		printf("Could not load %s file! \n Leaving the program....\n", filename);
		exit(-1);
	}

	struct Customer* addedCustomer = addNewCustomer();

	while (1) {
		int item_code;
		fscanf_s(customerFile, "%d", &item_code);
		if (item_code == 0) break;
		int qnt;
		fscanf_s(customerFile, "%d", &qnt);
		struct ItemStruct* purchasedItem = getProductScanned(item_code, qnt);
		addItemInCart(addedCustomer, purchasedItem);
	}

}

void sortList(struct ItemStruct* head) {
	if (head == NULL) return;
	struct ItemStruct* start;
	struct ItemStruct* temp = NULL;
	int isSwapped;
	do {
		isSwapped = 0;
		start = head;
		while (start->nextItem != temp) {
			if (start->ItemCode > start->nextItem->ItemCode) {
				swapProducts(start, start->nextItem);
				isSwapped = 1;
			}
			start = start->nextItem;
		}
		temp = start;
	} while (isSwapped);
}

void swapProducts(struct ItemStruct* one, struct ItemStruct* two) {
	int ItemCode = one->ItemCode;
	one->ItemCode = two->ItemCode;
	two->ItemCode = ItemCode;

	int NumItems = one->NumItems;
	one->NumItems = two->NumItems;
	two->NumItems = NumItems;

	float ItemPrice = one->ItemPrice;
	one->ItemPrice = two->ItemPrice;
	two->ItemPrice = ItemPrice;


	float TotSales = one->TotSales;
	one->TotSales = two->TotSales;
	two->TotSales = TotSales;

	char ItemName[17];
	strcpy(ItemName, one->ItemName);
	strcpy(one->ItemName, two->ItemName);
	strcpy(two->ItemName, ItemName);
}

struct ItemStruct* getProductScanned(int item_code,int qnt) {
	int category_index = ((item_code / 100) - 1);
	struct ItemStruct* head = categories[category_index].category_items;
	if (head == NULL) return NULL;
	
	while (head != NULL) {
		//printf("|%d| CHECK |%d| \n", head->ItemCode, item_code);	
		if (head->ItemCode == item_code) {
			head->TotSales += (head->ItemPrice * qnt);
			head->NumItems += qnt;

			struct ItemStruct* scannedProduct = (struct ItemStruct*)malloc(1 * sizeof(struct ItemStruct));
			if (scannedProduct == NULL) return NULL;
			scannedProduct->ItemCode = head->ItemCode;
			scannedProduct->NumItems = qnt;
			scannedProduct->ItemPrice = head->ItemPrice;
			scannedProduct->TotSales = (head->ItemPrice * qnt);
			strcpy(scannedProduct->ItemName,head->ItemName);
			scannedProduct->nextItem = NULL;
			return scannedProduct;
		}
		head = head->nextItem;
	}
	return NULL;
}

struct Customer* addNewCustomer() {
	struct Customer* head = customers;

	if (head == NULL) {
		head = (struct Customer*)malloc(1 * sizeof(struct Customer));
		if (head == NULL) {
			printf("Could not allocate memory..!");
			return;
		}
		head->customer_id = ++totalCustomers;
		head->cart = NULL;
		head->nextCustomer = customers;
		customers = head;
		return head;
	}
	else {
		while (head->nextCustomer != NULL)
			head = head->nextCustomer;

		struct Customer* newCustomer = (struct Customer*)malloc(1 * sizeof(struct Customer));
		if (newCustomer == NULL) {
			printf("Could not allocate memory..!");
			return;
		}
		newCustomer->customer_id = ++totalCustomers;
		newCustomer->cart = NULL;
		newCustomer->nextCustomer = NULL;
		head->nextCustomer = newCustomer;
		return newCustomer;
	}
	return NULL;
}

void addItemInCart(struct Customer* customer, struct ItemStruct* item) {
	struct ItemStruct* cart = customer->cart;

	if (cart == NULL) {
		//printf("Adding in head.... |%d| |%s|", item->ItemCode, item->ItemName);
		cart = (struct ItemStruct*)malloc(1 * sizeof(struct ItemStruct));
		if (cart == NULL) {
			printf("Could not allocate memory..!");
			return;
		}
		cart->ItemCode = item->ItemCode;
		strcpy(cart->ItemName, item->ItemName);
		cart->ItemPrice = item->ItemPrice;
		cart->TotSales = (item->ItemPrice* item->NumItems);
		cart->NumItems = item->NumItems;
		cart->nextItem = customer->cart;
		customer->cart = cart;
	}
	else {
		while (cart->nextItem != NULL)
			cart = cart->nextItem;
		struct ItemStruct* newItem = (struct ItemStruct*)malloc(1 * sizeof(struct ItemStruct));
		if (newItem == NULL) {
			printf("Could not allocate memory..!");
			return;
		}
		newItem->ItemCode = item->ItemCode;
		strcpy(newItem->ItemName, item->ItemName);
		newItem->ItemPrice = item->ItemPrice;
		newItem->TotSales = item->TotSales;
		newItem->NumItems = item->NumItems;
		newItem->nextItem = NULL;

		cart->nextItem = newItem;
	}
}

void printCustomerReceipts() {
	struct Customer* tempCustomers = customers;

	while (tempCustomers != NULL) {
		printf("\nCustomer Receipt # %d\n", tempCustomers->customer_id);
		printf("Code \t Item Name \t Price\tNumItem\tTotSale \n");
		int totalItems = 0;
		float totalSales = 0;
		struct ItemStruct* cart = tempCustomers->cart;
		while (cart != NULL) {
			printf(" %d\t%10.20s\t%.2f\t%d\t%.2f\n", cart->ItemCode, cart->ItemName, cart->ItemPrice,
				cart->NumItems, cart->TotSales);
			totalItems += cart->NumItems;
			totalSales += cart->TotSales;
			cart = cart->nextItem;
		}
		printf("%*s", 20,"");
		printf("Totals:\t%d\t%.2f \n", totalItems, totalSales);
		tempCustomers = tempCustomers->nextCustomer;
	}
}

void printCategoryInventory() {

	for (int i = 0; i < totalCategories; i++) {
		struct ItemStruct* tempCat = categories[i].category_items;
		int totalItems = 0;
		float totalSales = 0;

		printf("\nCategory Name: %s\n", categories[i].category_name);
		printf("Category Code: %d\n", categories[i].category_code);
		printf("Code \t Item Name \t NumItem\tTotSale \n");
		char numb[5];
		sprintf(numb, "%d", i * 100);
		char fileName[100];
		snprintf(fileName, sizeof fileName, "Invetory%d00.dat", (i+1));
		FILE* inventoryFile = fopen(fileName,"w");

		while (tempCat != NULL) {
			printf(" %d\t%10.20s\t%d\t%.2f\n", tempCat->ItemCode, tempCat->ItemName,
				tempCat->NumItems, tempCat->TotSales);
			fprintf(inventoryFile," %d\t%10.20s\t %d\t%.2f\n", tempCat->ItemCode, tempCat->ItemName,
				tempCat->NumItems, tempCat->TotSales);
			totalItems += tempCat->NumItems;
			totalSales += tempCat->TotSales;
			tempCat = tempCat->nextItem;
		}
		printf("Total Items Sold:\t%d\n", totalItems);
		printf("Total Sales:\t%f\n", totalSales);

	}
}

void dailySummaryReport() {
	printf("\n\nDaily Summary Report\n");
	printf("Code \t Category Name \t #Items Sold  	Tot Sales Amt \n");
	int totalItems = 0;
	float totalSales = 0;
	for (int i = 0; i < totalCategories; i++) {
		int items = 0;
		float sales = 0;
		struct ItemStruct* temp = categories[i].category_items;
		while (temp != NULL) {
			items += temp->NumItems;
			totalItems += items;
			sales += temp->TotSales;
			totalSales += sales;
			temp = temp->nextItem;
		}
		printf(" %d\t%10.20s\t\t%d\t%.2f\n", categories[i].category_code, categories[i].category_name,
			items, sales);
	}
	printf("\nTotal customers\t%d\n", totalCustomers);
	printf("Total Items sold\t%d\n", totalItems);
	printf("Total Sales\t%f\n", totalSales);
}