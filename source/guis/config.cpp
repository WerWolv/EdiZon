#include "helpers/config.hpp"

static Config::config_data_t configData;

void Config::readConfig() {
memset(&configData, 0x00, sizeof(config_data_t));

if (access(CONFIG_PATH, F_OK) == 0) {
    FILE *configFile = fopen(CONFIG_PATH, "r+");
    fread(&configData, 1, sizeof(config_data_t), configFile);
    fclose(configFile);

    if (strcmp(configData.magic, "EDZNCFG") != 0) {
    memset(&configData, 0x00, sizeof(config_data_t));
    Config::writeConfig();
    }
} else
    Config::writeConfig();
}

void Config::writeConfig() {
FILE *configFile = fopen(CONFIG_PATH, "wr");

memcpy(configData.magic, "EDZNCFG", 8);
fwrite(&configData, 1, sizeof(config_data_t), configFile);

fclose(configFile);
}

Config::config_data_t* Config::getConfig() {
return &configData;
}