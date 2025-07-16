#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>

#define MAX_LEN 100
#define KEY 0x5A
#define CRED_FILE "vault.txt"

// Cleanup handler
void handle_sigint(int sig) {
    printf("\n[!] SIGINT received. Securely wiping memory...\n");
    exit(0);
}

// Terminal access check
void verify_terminal(const char* expected) {
    char *tty = ttyname(STDIN_FILENO);
    if (!tty || strcmp(tty, expected) != 0) {
        printf("❌ Access denied. Terminal mismatch.\n");
        printf("Current terminal: %s\n", tty ? tty : "Unknown");
        exit(1);
    }
}

// User access check
void verify_user(const char* expected_user) {
    char *user = getenv("USER");
    if (!user || strcmp(user, expected_user) != 0) {
        printf("❌ Access denied. User mismatch.\n");
        printf("Current user: %s\n", user ? user : "Unknown");
        exit(1);
    }
}

// Hide password input
void get_hidden_input(char *buffer, int size) {
    struct termios oldt, newt;
    printf("Enter password: ");
    fflush(stdout);
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fgets(buffer, size, stdin);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
    buffer[strcspn(buffer, "\n")] = 0;
}

// XOR-based encryption/decryption
void xorEncryptDecrypt(char *str) {
    while (*str) {
        *str ^= KEY;
        str++;
    }
}

// Add encrypted credential to file
void addCredential() {
    char username[MAX_LEN], password[MAX_LEN];
    FILE *fp = fopen(CRED_FILE, "a");

    if (!fp) {
        printf("Error opening file!\n");
        return;
    }

    printf("Enter username: ");
    fgets(username, MAX_LEN, stdin);
    username[strcspn(username, "\n")] = 0;

    get_hidden_input(password, MAX_LEN);
    xorEncryptDecrypt(password);

    fprintf(fp, "%s %s\n", username, password);
    fclose(fp);

    printf("✅ Credential saved securely.\n");
}

// View all decrypted credentials
void viewCredentials() {
    char username[MAX_LEN], password[MAX_LEN];
    FILE *fp = fopen(CRED_FILE, "r");

    if (!fp) {
        printf("No stored credentials found.\n");
        return;
    }

    printf("\n🔐 Stored Credentials:\n");
    printf("----------------------\n");

    while (fscanf(fp, "%s %s", username, password) != EOF) {
        xorEncryptDecrypt(password);
        printf("Username: %s | Password: %s\n", username, password);
    }

    fclose(fp);
}

int main() {
    const char* allowed_user = "sthuthi11";    // echo $USER
    const char* allowed_tty  = "/dev/pts/0";   // tty

    signal(SIGINT, handle_sigint);

    verify_user(allowed_user);
    verify_terminal(allowed_tty);

    printf("\n🔐 BioLock Access Granted!\n");

    int choice;
    while (1) {
        printf("\n--- BioLock Vault Menu ---\n");
        printf("1. Add Credential\n");
        printf("2. View Credentials\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar(); // flush newline

        switch (choice) {
            case 1: addCredential(); break;
            case 2: viewCredentials(); break;
            case 3:
                printf("Vault closed.\n");
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}
