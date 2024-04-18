#include "headers/TextDB.h"

std::vector<TextStruct> TextDB::textQueue;
std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> TextDB::fontCache;
extern std::string gamePlaying;

TextDB::TextDB() {
    if (TTF_Init() == -1) {
        std::cout << "TTF_Init: " << TTF_GetError();
        exit(0);
    }
}

TextDB::~TextDB() {
    Clear();
    TTF_Quit();
}


void TextDB::LoadFont(const std::string& fontName, int fontSize) {
    std::string fontPath = "resources/" + gamePlaying + "/fonts/" + fontName + ".ttf";
    checkFontExists(fontPath);
    if (fontCache[fontName].find(fontSize) == fontCache[fontName].end()) {
        TTF_Font* font = TTF_OpenFont(fontPath.c_str(), fontSize);
        if (font == nullptr) {
            std::cout << "error: font " << fontName << " with size " << fontSize << " missing" << std::endl;
            exit(0);
        }
        fontCache[fontName][fontSize] = font;
    }
}

TTF_Font* TextDB::GetFont(const std::string& fontName, int fontSize) {
    // Check if the font at this size already exists in the cache
    if (fontCache.find(fontName) != fontCache.end() && fontCache[fontName].find(fontSize) != fontCache[fontName].end()) {
        return fontCache[fontName][fontSize];
    }
    else {
        // If the font is not in the cache, load it
        LoadFont(fontName, fontSize);
        // After loading, it should be available in the cache
        return fontCache[fontName][fontSize];
    }
}

void TextDB::checkFontExists(const std::string& fontPath) {
    if (!std::filesystem::exists(fontPath)) {
        std::cout << "error: font " << fontPath << " missing";
        exit(0);
    }
}

void TextDB::Clear() {
    for (auto& fontBySize : fontCache) {
        for (auto& font : fontBySize.second) {
            TTF_CloseFont(font.second);
        }
    }
    fontCache.clear();
}

void TextDB::DrawText(const std::string& str_content, float x, float y, const std::string& fontName, int fontSize, float r, float g, float b, float a){
    // Convert RGBA and position to appropriate SDL types
    SDL_Color color = { static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a) };
    SDL_Rect position = { static_cast<int>(x), static_cast<int>(y), 0, 0 }; // Width and height will be set based on text content and font size

    // Retrieve or load the font
    TTF_Font* font = GetFont(fontName, fontSize);
    if (!font) {
        std::cerr << "Failed to load font: " << fontName << " with size: " << fontSize << std::endl;
        return; // Early return if font loading fails
    }

    // Create and queue the text structure for rendering
    TextStruct textStruct = { str_content, color, position, font };
    textQueue.push_back(textStruct);
}



void TextDB::RenderAllText(SDL_Renderer* renderer) {
    for (auto& ts : textQueue) {
        SDL_Surface* surface = TTF_RenderText_Solid(ts.font, ts.text.c_str(), ts.color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        ts.position.w = surface->w;
        ts.position.h = surface->h;
        SDL_RenderCopy(renderer, texture, NULL, &ts.position);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
    textQueue.clear();
}


std::vector<std::string> TextDB::getIntroTexts(const rapidjson::Document& document) {
    std::vector<std::string> introTexts;

    if (document.HasMember("intro_text") && document["intro_text"].IsArray()) {
        for (const auto& text : document["intro_text"].GetArray()) {
            if (text.IsString()) {
                std::string textString = text.GetString();
                introTexts.push_back(textString); // Add the text string to the vector
            }
        }
    }

    return introTexts;
}

void TextDB::checkAllIntroFontsExists(const rapidjson::Document& document) {
    if (!document.HasMember("font") && document.HasMember("intro_text")) {
        std::cout << "error: text render failed. No font configured";
        exit(0);
    }
    else if (document.HasMember("font") && document["font"].IsString()) {
        std::string fontName = document["font"].GetString();
        checkFontExists(fontName);
    }

}
