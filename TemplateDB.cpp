#include "TemplateDB.h"
#include "EngineUtils.h"

#include <filesystem>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

unordered_map<string, rapidjson::Document> TemplateDB::templates;

bool TemplateDB::LoadTemplates() {

    string dir = "resources/actor_templates/";

    if (!fs::exists(dir)) {
        return false;
    }

    for (auto& entry : fs::directory_iterator(dir)) {

        if (!entry.is_regular_file()) continue;

        string path = entry.path().string();
        string ext = entry.path().extension().string();

        if (ext != ".template") continue;

        rapidjson::Document t_doc;
        EngineUtils::ReadJsonFile(path, t_doc);

        string key = entry.path().stem().string();

        templates[key].CopyFrom(t_doc, templates[key].GetAllocator());
    }

    return true;
}

bool TemplateDB::HasTemplate(const string& name) {
    return templates.find(name) != templates.end();
}

const rapidjson::Document& TemplateDB::GetTemplate(const string& name) {
    return templates.at(name);
}