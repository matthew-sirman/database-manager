// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <iostream>
//#include <unistd.h>
#include <filesystem>
#include <regex>
#include <Windows.h>

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

#ifdef _WIN32

std::string getPassword(const std::string &prompt) {
    const char BACKSPACE = 8, RETURN = 13;

    std::string password;
    unsigned char ch = 0;

    std::cout << prompt;

    DWORD consoleMode;
    DWORD dwRead;

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

    GetConsoleMode(hIn, &consoleMode);
    SetConsoleMode(hIn, consoleMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

    while (ReadConsoleA(hIn, &ch, 1, &dwRead, NULL) && ch != RETURN) {
        if (ch == BACKSPACE) {
            if (password.length() != 0) {
                std::cout << "\b \b";
                password.resize(password.length() - 1);
            }
        } else {
            password += ch;
            std::cout << "*";
        }
    }

    std::cout << std::endl;

    SetConsoleMode(hIn, consoleMode);

    return password;
}

#else

std::string getPassword(const std::string &prompt) {
    // Get the password from the terminal. This will hide the input
    char *pw = getpass(prompt.c_str());
    size_t pwLen = strlen(pw);
}

#endif

uint256 getPasswordHash(const std::string &prompt) {
    std::string password = getPassword(prompt);

    uint256 pwHash = sha256((const uint8 *) password.c_str(), password.size());

    memset(password.data(), 0, password.size());

    return pwHash;
}

void setupServerKeys(std::filesystem::path metaFilePath) {
    metaFilePath = metaFilePath / "serverMeta.json";
    if (!std::filesystem::exists(metaFilePath)) {
        std::cerr << "Meta file path " << metaFilePath << " does not exist." << std::endl;
        return;
    }

    std::ifstream metaFile;
    metaFile.open(metaFilePath);

    nlohmann::json meta;

    try {
        metaFile >> meta;

        metaFile.close();
    }
    catch (nlohmann::json::parse_error &) {
        std::cerr << "Failed to load " << metaFilePath << ". Check the JSON is valid" << std::endl;

        metaFile.close();
        return;
    }

    if (meta.find("keyPath") == meta.end()) {
        std::cerr << "Could not find 'keyPath' in meta file." << std::endl;
        return;
    }
    if (meta.find("databasePasswordPath") == meta.end()) {
        std::cerr << "Could not find 'databasePasswordPath' in meta file." << std::endl;
        return;
    }

    std::filesystem::path keyPath = meta["keyPath"].get<std::string>();
    std::filesystem::path databasePasswordPath = meta["databasePasswordPath"].get<std::string>();

    std::string databasePassword = getPassword("Enter database password: ");

    std::cout << "Generating RSA key pairs for the server and signatures (this may take up to a minute)..."
              << std::endl;

    // Generate the two keys for the server and the digital signature
    RSAKeyPair serverKeyPair = generateRSAKeyPair();
    DigitalSignatureKeyPair digitalSignatureKeyPair = generateDigitalSignatureKeyPair();

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

    lockData((uint8 *) databasePassword.c_str(), databasePassword.size(), databasePasswordPath / "encrypted_root.pass", pwHash);

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
            lockData((uint8 *) databasePassword.c_str(), databasePassword.size(), databasePasswordPath / ("encrypted_" + uname + ".pass"), pwHash);

            std::cout << "Keys saved successfully for user " << uname << "." << std::endl;
        } else {
            break;
        }
    }
}

void addUser(const std::string &newUser, std::filesystem::path metaFilePath, const std::string &authUser) {
    metaFilePath = metaFilePath / "serverMeta.json";
    if (!std::filesystem::exists(metaFilePath)) {
        std::cerr << "Meta file path " << metaFilePath << " does not exist." << std::endl;
        return;
    }

    std::ifstream metaFile;
    metaFile.open(metaFilePath);

    nlohmann::json meta;

    try {
        metaFile >> meta;

        metaFile.close();
    }
    catch (nlohmann::json::parse_error &) {
        std::cerr << "Failed to load " << metaFilePath << ". Check the JSON is valid" << std::endl;

        metaFile.close();
        return;
    }

    if (meta.find("keyPath") == meta.end()) {
        std::cerr << "Could not find 'keyPath' in meta file." << std::endl;
        return;
    }
    if (meta.find("databasePasswordPath") == meta.end()) {
        std::cerr << "Could not find 'databasePasswordPath' in meta file." << std::endl;
        return;
    }

    std::filesystem::path keyPath = meta["keyPath"].get<std::string>();
    std::filesystem::path databasePasswordPath = meta["databasePasswordPath"].get<std::string>();

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

    RSAKeyPair serverKeyPair;
    DigitalSignatureKeyPair digitalSignatureKeyPair;

    serverKeyPair.privateKey = unlockPrivateKey<32>(keyPath / ("server/server_key_" + authUser + ".pri"), pwHash);
    serverKeyPair.publicKey = readPublicKey<32>(keyPath / ("server/server_key.pub"));
    digitalSignatureKeyPair.privateKey = unlockPrivateKey<24>(keyPath / ("signature/signature_" + authUser + ".pri"),
                                                          pwHash);
    digitalSignatureKeyPair.publicKey = readPublicKey<24>(keyPath / ("signature/signature.pub"));
    std::string databasePassword = unlockStringData(databasePasswordPath / ("encrypted_" + authUser + ".pass"), pwHash);

    if (serverKeyPair.privateKey.n == serverKeyPair.publicKey.n &&
        digitalSignatureKeyPair.privateKey.n == digitalSignatureKeyPair.publicKey.n) {

        prompt.str(std::string());
        prompt << "Enter a password for new user " << newUser << ": ";

        pwHash = getPasswordHash(prompt.str());

        lockPrivateKey(serverKeyPair.privateKey, keyPath / ("server/server_key_" + newUser + ".pri"), pwHash);
        lockPrivateKey(digitalSignatureKeyPair.privateKey, keyPath / ("signature/signature_" + newUser + ".pri"),
                       pwHash);
        lockData((uint8 *) databasePassword.c_str(), databasePassword.size(), databasePasswordPath / ("encrypted_" + newUser + ".pass"), pwHash);

        std::cout << "Keys saved successfully for user " << newUser << "." << std::endl;
    } else {
        std::cerr << "Invalid password for user " << authUser << "." << std::endl;
        return;
    }
}

void runServer(std::filesystem::path metaFilePath, const std::string &user, bool dev) {
    metaFilePath = metaFilePath / "serverMeta.json";
    if (!std::filesystem::exists(metaFilePath)) {
        std::cerr << "Meta file path " << metaFilePath << " does not exist." << std::endl;
        return;
    }

    std::ifstream metaFile;
    metaFile.open(metaFilePath);

    nlohmann::json meta;

    try {
        metaFile >> meta;

        metaFile.close();
    }
    catch (nlohmann::json::parse_error &) {
        std::cerr << "Failed to load " << metaFilePath << ". Check the JSON is valid" << std::endl;

        metaFile.close();
        return;
    }

    if (meta.find("keyPath") == meta.end()) {
        std::cerr << "Could not find 'keyPath' in meta file." << std::endl;
        return;
    }
    if (meta.find("databasePasswordPath") == meta.end()) {
        std::cerr << "Could not find 'databasePasswordPath' in meta file." << std::endl;
        return;
    }
    if (meta.find("serverPort") == meta.end()) {
        std::cerr << "Could not find 'serverPort' in meta file." << std::endl;
        return;
    }
    if (meta.find("backupPath") == meta.end()) {
        std::cerr << "Could not file 'backupPath' in meta file." << std::endl;
        return;
    }

    std::filesystem::path keyPath = meta["keyPath"].get<std::string>();
    std::filesystem::path databasePasswordPath = meta["databasePasswordPath"].get<std::string>();
    unsigned serverPort = meta["serverPort"];
    std::filesystem::path backupPath = meta["backupPath"].get<std::string>();

    if (!std::filesystem::exists(keyPath / ("server/server_key_" + user + ".pri")) ||
        !std::filesystem::exists(keyPath / ("server/server_key.pub")) ||
        !std::filesystem::exists(keyPath / ("signature/signature_" + user + ".pri")) ||
        !std::filesystem::exists(keyPath / ("signature/signature.pub"))) {
        std::cerr << "There is no key file associated with user " << user << "." << std::endl;
        return;
    }

    if (!dev) {
        std::cout << "Live Mode" << std::endl;
    }
    else {
        std::cout << "DEV MODE" << std::endl;
    }

    // Create the prompt
    std::stringstream prompt;
    prompt << "Enter the password for " << user << ": ";

    // Hash the password to use as an AES key to secure the two keys
    uint256 pwHash = getPasswordHash(prompt.str());

    RSAKeyPair serverKeyPair;
    DigitalSignatureKeyPair digitalSignatureKeyPair;

    serverKeyPair.privateKey = unlockPrivateKey<32>(keyPath / ("server/server_key_" + user + ".pri"), pwHash);
    serverKeyPair.publicKey = readPublicKey<32>(keyPath / ("server/server_key.pub"));
    digitalSignatureKeyPair.privateKey = unlockPrivateKey<24>(keyPath / ("signature/signature_" + user + ".pri"), pwHash);
    digitalSignatureKeyPair.publicKey = readPublicKey<24>(keyPath / ("signature/signature.pub"));
    std::string databasePassword = unlockStringData(databasePasswordPath / ("encrypted_" + user + ".pass"), pwHash);

    if (serverKeyPair.privateKey.n != serverKeyPair.publicKey.n ||
        digitalSignatureKeyPair.privateKey.n != digitalSignatureKeyPair.publicKey.n) {
        std::cerr << "Invalid password for user " << user << "." << std::endl;
        return;
    }

    std::ostream *errStream = &std::cerr, *changelogStream = nullptr, *logStream = &std::cout;

    if (meta.find("logFile") != meta.end()) {
        logStream = new std::ofstream(meta["logFile"].get<std::string>(), std::ios::app);
        std::cout << "Logging to: " << meta["logFile"] << std::endl;
    }
    if (meta.find("changelogFile") != meta.end()) {
        changelogStream = new std::ofstream(meta["changelogFile"].get<std::string>(), std::ios::app);
        std::cout << "Logging changes to: " << meta["changelogFile"] << std::endl;
    }
    if (meta.find("errorFile") != meta.end()) {
        errStream = new std::ofstream(meta["errorFile"].get<std::string>(), std::ios::app);
        std::cout << "Logging errors to: " << meta["errorFile"] << std::endl;
    }

    // Initialise a server object with a refresh rate of 16Hz
    Server s(16, serverKeyPair, digitalSignatureKeyPair);

    s.setLoggingStream(logStream, changelogStream, errStream);

    DatabaseRequestHandler handler;
    handler.backupPath = backupPath;

    s.initialiseServer(serverPort);

    /*std::ifstream dbPasswordFile(keyPath / "../db-pw.txt");
    std::string dbPassword;
    dbPasswordFile >> dbPassword;*/

    if (!dev) {
        s.connectToDatabaseServer("screen_mat_database", "db-server-user", databasePassword);
    }
    else {
        std::string databasePassword_dev = getPassword("Dev Password: ");
        s.connectToDatabaseServer("screen_mat_database_dev", "dev", databasePassword_dev, "scs.local");
    }

    s.setRequestHandler(handler);

    // Send a heartbeat approximately every minute
    s.setHeartBeatCycles(1024);

    // Start running the server - this will enter a loop and return when the server is closed
    s.startServer();

    delete logStream;
    delete changelogStream;
    delete errStream;
}

int runClient(const std::filesystem::path &clientMetaFile, int argc, char *argv[], bool console) {
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/scs_logo.png"));

    MainMenu mainMenu(clientMetaFile);
    mainMenu.showMaximized();

    return QApplication::exec();
}

void printHelpMessage() {
    // Print a help message to the console
    std::cout << "Database Manager Help" << std::endl;
    std::cout << "Flags: " << std::endl;
    std::cout << "  --server            - run the server." << std::endl;
    std::cout << "  --dev               - run the server on the dev database" << std::endl;
    std::cout << "  --client            - run a client instance." << std::endl;
    std::cout << "  --console           - runs client with console" << std::endl;
    std::cout << "  --setup             - set the server keys up and save them in key files." << std::endl;
    std::cout << "                        NOTE: This will not start the server." << std::endl;
    std::cout << "  --add-user [USER]   - add a new admin user and password. This user will be able to" << std::endl;
    std::cout << "                        start the server and unlock the key files." << std::endl;
    std::cout << "                        NOTE: The --user flag will specify the user to authenticate this new"
              << std::endl;
    std::cout << "                        user with. Defaults to root." << std::endl;
    std::cout << "  --meta [PATH]       - provide an explicit path to the meta file." << std::endl;
    std::cout << "                        This path should contain 'clientMeta.json' for running the client" << std::endl;
    std::cout << "                        or 'serverMeta.json' for running the server." << std::endl;
    std::cout << "  --user [USER]       - provide a username to unlock the key files with. If no" << std::endl;
    std::cout << "                        username is provided, it will load the keys from the root file." << std::endl;
    std::cout << "  --help (-h)         - prints this help message." << std::endl;
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    if (TCPSocket::initialiseWSA()) {
        return -1;
    }
#endif

    // The mode to run in. This changes depending on the arguments provided
    RunMode mode = NO_MODE;
    // Whether it is in normal or dev mode
    bool dev = false;
    // Whether to run with console
    bool console = false;
    // The user to start the server on
    std::string user = "root";
    // The new user if they are running in add user mode
    std::string newUser;
    // The path to the client's meta file
    std::filesystem::path metaFilePath = std::filesystem::current_path();

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
                    goto error;
                }
            }

            // If they used the flag "--run" they want to run the server.
            if (strcmp(argv[i], "--server") == 0) {
                if (mode == NO_MODE) {
                    mode = SERVER;
                } else {
                    std::cout << "Invalid arguments. Use --help for more information." << std::endl;
                    std::cerr << "ERROR: You can only use one mode at a time." << std::endl;
                    goto error;
                }
            }
            
            // If they used the flag "--dev" they want to run the server in dev mode.
            if (strcmp(argv[i], "--dev") == 0) {
                dev = true;
            }

            // If they used the flag "--client" they want to run a client
            if (strcmp(argv[i], "--client") == 0) {
                if (mode == NO_MODE) {
                    mode = CLIENT;
                } else {
                    std::cout << "Invalid arguments. Use --help for more information." << std::endl;
                    std::cerr << "ERROR: You can only use one more at a time." << std::endl;
                    goto error;
                }
            }

            if (strcmp(argv[i], "--console") == 0) {
                console = true;
            }

            if (strcmp(argv[i], "--add-user") == 0) {
                if (mode == NO_MODE) {
                    mode = ADD_USER;
                    newUser = argv[++i];

                    if (!std::regex_match(newUser, USERNAME_CHECK)) {
                        std::cerr << "Invalid username for add user. Use lowercase letters only." << std::endl;
                        goto error;
                    }
                } else {
                    std::cout << "Invalid arguments. Use --help for more information." << std::endl;
                    std::cerr << "ERROR: You can only use one mode at a time." << std::endl;
                    goto error;
                }
            }

            if (strcmp(argv[i], "--meta") == 0) {
                metaFilePath = argv[++i];
            }

            // If they provided a specific username to load the key files with, use that. Default to root.
            if (strcmp(argv[i], "--user") == 0) {
                user = argv[++i];
            }
        }
    }
    if (!console && mode != SERVER) {
        FreeConsole();
        PostMessage(GetConsoleWindow(), WM_CLOSE, 0, 0);
    }

    if (dev && mode != SERVER) {
        std::cout << "Invalid arguments. Use --help for more information." << std::endl;
        std::cerr << "Only the server can be in dev mode." << std::endl;
        goto error;
    }

    switch (mode) {
        case SERVER:
            runServer(metaFilePath, user, dev);
            break;
        case CLIENT:
            return runClient(metaFilePath, argc, argv, console);
        case SETUP:
            setupServerKeys(metaFilePath);
            break;
        case ADD_USER:
            addUser(newUser, metaFilePath, user);
            break;
        case HELP:
            printHelpMessage();
            break;
        case NO_MODE:
            std::cout << "Invalid arguments. Use --help for more information." << std::endl;
            std::cerr << "ERROR: No mode selected." << std::endl;
            goto error;
    }

#ifdef _WIN32
    TCPSocket::cleanupWSA();
#endif

    return 0;

error:

#ifdef _WIN32
    TCPSocket::cleanupWSA();
#endif

    return -1;
}
