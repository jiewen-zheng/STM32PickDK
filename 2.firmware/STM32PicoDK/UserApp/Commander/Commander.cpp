//
// Created by moon on 2023/10/29.
//

#include <cstring>
#include "stdlib.h"
#include "Commander.h"
#include "Commands.h"

Commander::Commander(size_t _size, char midl, char eol, bool echo) :
        FifoArray<char>(_size) {
    this->midl = midl;
    this->eol  = eol;
    this->echo = echo;
}

void Commander::add(const char *id, CommandCallback onCommand, const char *label) {
    commandList[cmd_count].id        = (char *) id;
    commandList[cmd_count].onCommand = onCommand;
    commandList[cmd_count].label     = (char *) label;
    cmd_count++;
}

void Commander::run() {
    while (available()) {
        int ch = read();
        receivedBuff[rec_cnt++] = (char) ch;

        if (echo)
            print(ch);
        if (isEnds(ch)) {
            inputCheck(receivedBuff);

            // reset the command buffer
            memset(receivedBuff, 0, MAX_COMMAND_LENGTH);
//            receivedBuff[0] = 0;
            rec_cnt = 0;
        }
        if (rec_cnt >= MAX_COMMAND_LENGTH) {
            receivedBuff[0] = 0;
            rec_cnt = 0;
        }
    }
}

void Commander::scalar(float *value, char *user_cmd) {
    bool ends = isEnds(user_cmd[0]);
//    if(!ends)
    *value = (float) atof(user_cmd);
//    println(*value);
}

void Commander::str(char *str, char *user_cmd) {
    strcpy(str, user_cmd);
}

void Commander::inputCheck(char *input) {
    char *ptr = strchr(input, midl);

    if (ptr != nullptr) {
        int id_len = ptr - input;

        for (int i = 0; i < cmd_count; i++) {
            if (strncmp(input, commandList[i].id, id_len) == 0) {
                if (commandList[i].label) println(commandList[i].label);
                commandList[i].onCommand(ptr + 1);
                return;
            }
        }
        return;
    }

    char id = input[0];
    switch (id) {
        case CMD_SCAN:
            println(CMD_SCAN);
            for (int i = 0; i < cmd_count; i++) {
                print("[");
                print(commandList[i].id);
                print("]: ");
                if (commandList[i].label) println(commandList[i].label);
                else println("");
            }
            break;

        case CMD_VERBOSE:
            break;

        default:
            for (int i = 0; i < cmd_count; i++) {
                int arg_pos = strlen(commandList[i].id);

                if (strncmp(input, commandList[i].id, arg_pos) == 0) {
                    if (commandList[i].label) println(commandList[i].label);

                    if (strlen(input) > arg_pos + 1)
                        commandList[i].onCommand(input + arg_pos + 1);
                    else
                        commandList[i].onCommand(nullptr);
                    break;
                }
            }
            break;
    }

}

bool Commander::isEnds(char ch) {
    if (ch == eol)
        return true;
    else if (ch == '\r') {
        println("Warn: \\r detected!");
        return true;
    }
    return false;
}

void Commander::print(const char msg) {
    CMD_PRINT("%c", msg);
}

void Commander::print(const char *msg) {
    CMD_PRINT("%s", msg);
}

void Commander::println(char msg) {
    CMD_PRINT("%c\r\n", msg);
}

void Commander::println(float msg) {
    CMD_PRINT("%f\r\n", msg);
}

void Commander::println(const char *msg) {
    CMD_PRINT("%s\r\n", msg);
}






