#include "headers/ImageDB.h"

std::unordered_map<std::string, SDL_Texture*> ImageDB::imageCache;
std::vector<RenderRequest> ImageDB::requests;
extern std::string gamePlaying;

ImageDB::ImageDB() {
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError();
        exit(0);
    }
}

ImageDB::~ImageDB() {
    clear(); // Ensure all textures are freed  
}

SDL_Texture* ImageDB::loadImage(const std::string& imageName, SDL_Renderer* renderer) {
    // Check if the image is already loaded
    auto it = imageCache.find(imageName);
    if (it != imageCache.end()) {
        return it->second; // Return the cached texture
    }

    // Image path
    std::string imagePath = "resources/" + gamePlaying + "/images/" + imageName + ".png";

  
    // Load the image into a texture
    SDL_Texture* texture = IMG_LoadTexture(renderer, imagePath.c_str());
    if (!texture) {
        std::cout << "error: missing image " << imageName;
        exit(0);
    } 

    // Store the texture in the cache and return it
    imageCache[imageName] = texture; 
    return texture;
}

void ImageDB::clear() {
    // Iterate through the map and destroy each texture
    for (auto& pair : imageCache) {
        SDL_DestroyTexture(pair.second);
    }
    imageCache.clear(); // Clear the map
}

void ImageDB::checkImageExists(const std::string& imageName) {
    std::filesystem::path imagePath{ "resources/" + gamePlaying + "/images/" + imageName + ".png"};
    if (!std::filesystem::exists(imagePath)) {
        std::cout << "error: missing image " << imageName;
        exit(0);
    }
}

void ImageDB::DrawUI(std::string imageName, float x, float y) {
    checkImageExists(imageName);
    RenderRequest request(RenderRequest::ImageType::UI, imageName, x, y, 0);
    requests.push_back(request);
}

void ImageDB::DrawUIEx(std::string imageName, float x, float y, float r, float g, float b, float a, int sorting_order) {
   
    checkImageExists(imageName);
    RenderRequest request(RenderRequest::ImageType::UI, imageName, x, y, sorting_order);
    // Set color and alpha values
    request.r = static_cast<int>(std::floor(r));
    request.g = static_cast<int>(std::floor(g));
    request.b = static_cast<int>(std::floor(b));
    request.a = static_cast<int>(std::floor(a));
    // Queue the request
    requests.push_back(request);
}


void ImageDB::Draw(std::string imageName, float x, float y) {
    // Check if the image is loaded, load it if not
    checkImageExists(imageName);
    // Create a RenderRequest for the scene image
    RenderRequest request(RenderRequest::ImageType::Scene, imageName, x, y);
    // Queue the request
    requests.push_back(request);
}

void ImageDB::DrawEx(std::string imageName, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, int sorting_order) {
    // Ensure the image is in the cache
    checkImageExists(imageName);

    // Create an extended RenderRequest with all properties
    RenderRequest request(RenderRequest::ImageType::Scene, imageName, x, y, sorting_order);
    request.rotation_degrees = static_cast<int>(rotation_degrees);
    request.scale_x = scale_x;
    request.scale_y = scale_y;
    request.pivot_x = pivot_x;
    request.pivot_y = pivot_y;
    request.r = static_cast<int>(r);
    request.g = static_cast<int>(g);
    request.b = static_cast<int>(b);
    request.a = static_cast<int>(a);

    // Queue the request
    requests.push_back(request);
}

void ImageDB::DrawPixel(float x, float y, float r, float g, float b, float a) {
    RenderRequest request(RenderRequest::ImageType::Pixel, "", x, y);
    request.r = static_cast<int>(r);
    request.g = static_cast<int>(g);
    request.b = static_cast<int>(b);
    request.a = static_cast<int>(a);

    requests.push_back(request);
}

void ImageDB::clearAll() {
    imageCache.clear(); requests.clear();
}