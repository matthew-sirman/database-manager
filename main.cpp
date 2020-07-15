#include <iostream>
//#include <unistd.h>
#include <filesystem>
#include <regex>

#include <QApplication>

#include <encrypt.h>
#include "include/networking/Server.h"
#include "include/networking/Client.h"
#include "include/database/DatabaseRequestHandler.h"
#include "ui/MainMenu.h"

enum RunMode {
    NO_MODE,
    SERVER,
    CLIENT,
    SETUP,
    ADD_USER,
    HELP
};

#define USERNAME_CHECK std::regex("^[a-z]+$")

uint256 getPasswordHash(const std::string &prompt) {
    // Get the password from the terminal. This will hide the input
    char *pw = getpass(prompt.c_str());
    size_t pwLen = strlen(pw);

    // Hash the password to use as an AES key to secure the two keys
    uint256 pwHash = sha256((const uint8 *) pw, pwLen);

    // Overwrite the password buffer with all 0s
    memset(pw, 0, pwLen);

    return pwHash;
}

void setupServerKeys(const std::filesystem::path &keyPath) {
    if (!std::filesystem::exists(keyPath)) {
        std::cerr << "Key path " << keyPath << " does not exist." << std::endl;
        return;
    }

    std::cout << "Generating RSA key pairs for the server and signatures (this may take up to a minute)..."
              << std::endl;

    // Generate the two keys for the server and the digital signature
    RSAKeyPair serverKeyPair = generateRSAKeyPair();
    RSAKeyPair digitalSignatureKeyPair = generateRSAKeyPair();

    std::cout << "Keys generated." << std::endl;
    std::cout << "Enter one or more passwords to save the encryption key files with." << std::endl;
    std::cout << "Any one of these passwords will be needed to decrypt the key file for future use." << std::endl;

    // Hash the password to use as an AES key to secure the two keys
    uint256 pwHash = getPasswordHash("Enter a root password to secure the encryption keys under: ");

    std::filesystem::create_directory(keyPath / "server");
    std::filesystem::create_directory(keyPath / "signature");

    lockPrivateKey(serverKeyPair.privateKey, keyPath / "server/server_key_root.pri", pwHash);
    writePublicKey(serverKeyPair.publicKey, keyPath / "server/server_key.pub");
    lockPrivateKey(digitalSignatureKeyPair.privateKey, keyPath / "signature/signature_root.pri", pwHash);
    writePublicKey(digitalSignatureKeyPair.publicKey, keyPath / "signature/signature.pub");

    std::cout << "Keys saved successfully for user root." << std::endl;

    while (true) {
        std::cout << std::endl;
        std::cout << "Would you like to enter another password? [y/N] ";
        std::string response;
        std::cin >> response;

        if (response == "y" || response == "Y" || response == "yes") {
            // Get a username for this password
            std::string uname;
            std::cout << "Enter a username (lowercase letters only) to associate this password with: ";
            std::cin >> uname;

            if (!std::regex_match(uname, USERNAME_CHECK)) {
                std::cerr << "Invalid username. Use lowercase letters only." << std::endl;
                continue;
            }

            // Create the prompt
            std::stringstream prompt;
            prompt << "Enter a password for " << uname << ": ";

            pwHash = getPasswordHash(prompt.str());

            lockPrivateKey(serverKeyPair.privateKey, keyPath / ("server/server_key_" + uname + ".pri"), pwHash);
            lockPrivateKey(digitalSignatureKeyPair.privateKey, keyPath / ("signature/signature_" + uname + ".pri"),
                           pwHash);

            std::cout << "Keys saved successfully for user " << uname << "." << std::endl;
        } else {
            break;
        }
    }
}

void addUser(const std::string &newUser, const std::filesystem::path &keyPath, const std::string &authUser) {
    if (!std::filesystem::exists(keyPath)) {
        std::cerr << "Key path " << keyPath << " does not exist." << std::endl;
        return;
    }

    if (!std::filesystem::exists(keyPath / ("server/server_key_" + authUser + ".pri")) ||
        !std::filesystem::exists(keyPath / ("server/server_key.pub")) ||
        !std::filesystem::exists(keyPath / ("signature/signature_" + authUser + ".pri")) ||
        !std::filesystem::exists(keyPath / ("signature/signature.pub"))) {
        std::cerr << "There is no key file associated with user " << authUser << "." << std::endl;
        return;
    }

    // Create the prompt
    std::stringstream prompt;
    prompt << "Enter the password for " << authUser << ": ";

    // Hash the password to use as an AES key to secure the two keys
    uint256 pwHash = getPasswordHash(prompt.str());

    RSAKeyPair serverKeyPair, digitalSignatureKeyPair;

    serverKeyPair.privateKey = unlockPrivateKey(keyPath / ("server/server_key_" + authUser + ".pri"), pwHash);
    serverKeyPair.publicKey = readPublicKey(keyPath / ("server/server_key.pub"));
    digitalSignatureKeyPair.privateKey = unlockPrivateKey(keyPath / ("signature/signature_" + authUser + ".pri"),
                                                          pwHash);
    digitalSignatureKeyPair.publicKey = readPublicKey(keyPath / ("signature/signature.pub"));

    if (serverKeyPair.privateKey.n == serverKeyPair.publicKey.n &&
        digitalSignatureKeyPair.privateKey.n == digitalSignatureKeyPair.publicKey.n) {

        prompt.str(std::string());
        prompt << "Enter a password for new user " << newUser << ": ";

        pwHash = getPasswordHash(prompt.str());

        lockPrivateKey(serverKeyPair.privateKey, keyPath / ("server/server_key_" + newUser + ".pri"), pwHash);
        lockPrivateKey(digitalSignatureKeyPair.privateKey, keyPath / ("signature/signature_" + newUser + ".pri"),
                       pwHash);

        std::cout << "Keys saved successfully for user " << newUser << "." << std::endl;
    } else {
        std::cerr << "Invalid password for user " << authUser << "." << std::endl;
        return;
    }
}

void runServer(const std::filesystem::path &keyPath, const std::string &user) {
    if (!std::filesystem::exists(keyPath)) {
        std::cerr << "Key path " << keyPath << " does not exist." << std::endl;
        return;
    }

    if (!std::filesystem::exists(keyPath / ("server/server_key_" + user + ".pri")) ||
        !std::filesystem::exists(keyPath / ("server/server_key.pub")) ||
        !std::filesystem::exists(keyPath / ("signature/signature_" + user + ".pri")) ||
        !std::filesystem::exists(keyPath / ("signature/signature.pub"))) {
        std::cerr << "There is no key file associated with user " << user << "." << std::endl;
        return;
    }

    // TODO: Swap this over
//    // Create the prompt
//    std::stringstream prompt;
//    prompt << "Enter the password for " << user << ": ";
//
//    // Hash the password to use as an AES key to secure the two keys
//    uint256 pwHash = getPasswordHash(prompt.str());

    std::string tpw = "password";
    uint256 pwHash = sha256(tpw.c_str(), tpw.size());

    RSAKeyPair serverKeyPair, digitalSignatureKeyPair;

    serverKeyPair.privateKey = unlockPrivateKey(keyPath / ("server/server_key_" + user + ".pri"), pwHash);
    serverKeyPair.publicKey = readPublicKey(keyPath / ("server/server_key.pub"));
    digitalSignatureKeyPair.privateKey = unlockPrivateKey(keyPath / ("signature/signature_" + user + ".pri"), pwHash);
    digitalSignatureKeyPair.publicKey = readPublicKey(keyPath / ("signature/signature.pub"));

    if (serverKeyPair.privateKey.n != serverKeyPair.publicKey.n ||
        digitalSignatureKeyPair.privateKey.n != digitalSignatureKeyPair.publicKey.n) {
        std::cerr << "Invalid password for user " << user << "." << std::endl;
        return;
    }

    // Initialise a server object with a refresh rate of 16Hz
    Server s(16, serverKeyPair, digitalSignatureKeyPair);

    DatabaseRequestHandler handler;

    // Start the server with the given port, TODO: fix which port rather than constant 10000, dynamic password
    s.initialiseServer(10000);

    std::ifstream dbPasswordFile(keyPath / "db-manager-server-mysql-pw");
    std::string dbPassword;
    dbPasswordFile >> dbPassword;

    s.connectToDatabaseServer("scs_drawings", "db-server-user", dbPassword);

    s.setRequestHandler(handler);

    // Send a heartbeat approximately every minute
    s.setHeartBeatCycles(64);

    // Start running the server - this will enter a loop and return when the server is closed
    s.startServer();
}

int runClient(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainMenu mainMenu;
    mainMenu.show();

    return QApplication::exec();
}

void printHelpMessage() {
    // Print a help message to the console
    std::cout << "Database Manager Help" << std::endl;
    std::cout << "Flags: " << std::endl;
    std::cout << "  --server             - run the server." << std::endl;
    std::cout << "  --client          - run a client instance." << std::endl;
    std::cout << "  --setup           - set the server keys up and save them in key files." << std::endl;
    std::cout << "                      NOTE: This will not start the server." << std::endl;
    std::cout << "  --add-user [USER] - add a new admin user and password. This user will be able to" << std::endl;
    std::cout << "                      start the server and unlock the key files." << std::endl;
    std::cout << "                      NOTE: The --user flag will specify the user to authenticate this new"
              << std::endl;
    std::cout << "                      user with. Defaults to root." << std::endl;
    std::cout << "  --keypath [PATH]  - provide an explicit path to store/load the key files." << std::endl;
    std::cout << "  --user [USER]     - provide a username to unlock the key files with. If no" << std::endl;
    std::cout << "                      username is provided, it will load the keys from the root file." << std::endl;
    std::cout << "  --help (-h)       - prints this help message." << std::endl;
}

int main(int argc, char *argv[]) {
    // The mode to run in. This changes depending on the arguments provided
    RunMode mode = NO_MODE;
    // The path to create the key files in
    std::filesystem::path keyPath = std::filesystem::current_path();
    // The user to start the server on
    std::string user = "root";
    // The new user if they are running in add user mode
    std::string newUser;

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            // If the user users the argument "-h" or "--help" set the mode to help and break instantly
            // (they want help; not to use the application in any way)
            if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                mode = HELP;
                break;
            }

            // If they used the flag "--setup" then they want to setup the server.
            if (strcmp(argv[i], "--setup") == 0) {
                if (mode == NO_MODE) {
                    mode = SETUP;
                } else {
                    std::cout << "Invalid arguments. Use --help for more information." << std::endl;
                    std::cerr << "ERROR: You can only use one mode at a time." << std::endl;
                    return -1;
                }
            }

            // If they used the flag "--run" they want to run the server.
            if (strcmp(argv[i], "--server") == 0) {
                if (mode == NO_MODE) {
                    mode = SERVER;
                } else {
                    std::cout << "Invalid arguments. Use --help for more information." << std::endl;
                    std::cerr << "ERROR: You can only use one mode at a time." << std::endl;
                    return -1;
                }
            }

            // If they used the flag "--client" they want to run a client
            if (strcmp(argv[i], "--client") == 0) {
                if (mode == NO_MODE) {
                    mode = CLIENT;
                } else {
                    std::cout << "Invalid arguments. Use --help for more information." << std::endl;
                    std::cerr << "ERRPR: You can only use one more at a time." << std::endl;
                }
            }

            if (strcmp(argv[i], "--add-user") == 0) {
                if (mode == NO_MODE) {
                    mode = ADD_USER;
                    newUser = argv[++i];

                    if (!std::regex_match(newUser, USERNAME_CHECK)) {
                        std::cerr << "Invalid username for add user. Use lowercase letters only." << std::endl;
                        return -1;
                    }
                } else {
                    std::cout << "Invalid arguments. Use --help for more information." << std::endl;
                    std::cerr << "ERROR: You can only use one mode at a time." << std::endl;
                    return -1;
                }
            }

            // If they specified a key path, use that to create the keys (setup only)
            if (strcmp(argv[i], "--keypath") == 0) {
                keyPath = argv[++i];
            }

            // If they provided a specific username to load the key files with, use that. Default to root.
            if (strcmp(argv[i], "--user") == 0) {
                user = argv[++i];
            }
        }
    }

    switch (mode) {
        case SERVER:
            runServer(keyPath, user);
            break;
        case CLIENT:
            return runClient(argc, argv);
        case SETUP:
            setupServerKeys(keyPath);
            break;
        case ADD_USER:
            addUser(newUser, keyPath, user);
            break;
        case HELP:
            printHelpMessage();
            break;
        case NO_MODE:
            std::cout << "Invalid arguments. Use --help for more information." << std::endl;
            std::cerr << "ERROR: No mode selected." << std::endl;
            return -1;
    }

    return 0;
}
