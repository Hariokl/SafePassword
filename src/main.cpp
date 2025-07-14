//TODO: hide all passwords, mb later add a button to reveal password
//TODO: make it, so all buttons, heights, widths and placement is connected to WIDTH and HEIGHT of the window (there should be relativity everywhere to WIDTH and HEIGHT)
//TODO: make this more universal code (for different systems) by adding specified int and char types like int8
//TODO: make this available for linux (?)
//TODO: make a better visuals altogether :D
//TODO: кнопка delete слишком вырежена, мб перенести в главный экран и сделать крестик
//TODO: make it so user can rearange services/accounts however they want
//TODO: to make a better code, mb instead of updating variables every loop, while not update them only when changes are made
//TODO: сделать словарь (структуру словарь), в котором будут хранится слова для кнопок на разных языках
//TODO: создать настройки, в которых можно будет настроить языки
//TODO: currently when wanting to exit the app when in accounts list, it just returns to services list (should be: when exiting the app it exits the app)
//TODO: add more font sizes
//TODO: currently in services list, search bar y position is relative to service list, but it should be the other way around
//TODO: make a separate folder near in root called app where new .exe and all things needed for it will be stored going forward
//TODO: make an algorithm that will encryp/decrypt save file
//TODO: add a message that password was copied
//TODO: add button to return from list of accounts to list of services at the top near service name
//TODO: make it so all fonts are in namespace and/or globally accessable

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <windows.h>

namespace {
const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 700;
const int MAX_CHARACTERS = 20;
const char PATH_SAVE[9] = "save.txt";

struct Account {
    std::string accountName;
    std::string password;

};

struct Service {
    std::string label;
    std::vector<Account> accounts;

};

struct MultiInputResult {
    bool submitted;
    std::vector<std::string> inputs;
};

void saveToFile(const std::vector<Service>& services, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    for (const auto& service : services) {
        for (const auto& account : service.accounts) {
            outFile << service.label << ";" << account.accountName << ";" << account.password << "\n";
        }
        if (service.accounts.size() == 0) {
            outFile << service.label << ";;";
        }
    }
    outFile.close();
}

void loadFromFile(std::vector<Service>& services, const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "No existing file to load: " << filename << std::endl;
        return;
    }
    std::unordered_map<std::string, Service*> serviceMap;
    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string serviceName, accountName, password;
        if (std::getline(ss, serviceName, ';') &&
            std::getline(ss, accountName, ';') &&
            std::getline(ss, password)) {

            if (serviceMap.find(serviceName) == serviceMap.end()) {
                services.emplace_back(Service{serviceName});
                serviceMap[serviceName] = &services.back();
            }
            serviceMap[serviceName]->accounts.push_back({accountName, password});
        } else if (serviceName.size() != 0) {
            if (serviceMap.find(serviceName) == serviceMap.end()) {
                services.emplace_back(Service{serviceName});
                serviceMap[serviceName] = &services.back();
            }
        }
    }
    inFile.close();
}

std::vector<const char*> extractServiceNames(const std::vector<Service>& services) {
    std::vector<const char*> names;
    for (const auto& service : services) {
        names.push_back(service.label.c_str());
    }
    names.push_back(nullptr); // null-terminated for SDL UI
    return names;
}

std::vector<const char*> extractAccountNames(const std::vector<Account>& accounts) {
    std::vector<const char*> names;
    for (const Account& account : accounts) {
        names.push_back(account.accountName.c_str());
    }
    names.push_back(nullptr); // null-terminated for SDL UI
    return names;
}

}

MultiInputResult getMultipleTextInput(SDL_Renderer* renderer, TTF_Font* font, int size, const char** placeholders, const char** names) {
    SDL_StartTextInput();

    std::vector<std::string> inputs(size, "");
    int activeInput = 0;
    bool done = false;
    bool canceled = false;

    SDL_Event e;

    SDL_Color boxColor = { 50, 50, 50, 255 };
    SDL_Color borderColor = { 255, 255, 255, 255 };
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Color placeholderColor = { 150, 150, 150, 255 };
    SDL_Color bgColor = { 40, 40, 40, 255 };
    SDL_Color buttonColor = { 70, 130, 180, 255 }; // steel blue
    SDL_Color buttonHoverColor = { 100, 160, 210, 255 };

    const int boxHeight = 50;
    const int spacing = 20;
    const int buttonHeight = 40;
    const int buttonWidth = 150;

    // Extract first word from placeholders[0]
    std::string labelText = placeholders[0];
    size_t spacePos = labelText.find(' ');
    if (spacePos != std::string::npos) {
        labelText = labelText.substr(0, spacePos);
    }
    // For fading warning message
    Uint32 messageTimer = 0;
    const Uint32 messageDuration = 2000; // milliseconds
    std::string warningMessage = "This " + labelText + " already exists";

    SDL_Surface* labelSurf = TTF_RenderText_Blended(font, labelText.c_str(), textColor);
    SDL_Texture* labelTex = SDL_CreateTextureFromSurface(renderer, labelSurf);
    int labelHeight = labelSurf->h;
    int warningMessageHeight = 0;
    {
        SDL_Surface* tmp = TTF_RenderText_Blended(font, warningMessage.c_str(), textColor);
        warningMessageHeight = tmp->h;
        SDL_FreeSurface(tmp);
    }


    // Compute block height to include inputs and button
    const int totalBoxHeight = (boxHeight + spacing) * size
                         + spacing + labelHeight
                         + spacing + warningMessageHeight
                         + spacing + buttonHeight
                         + spacing;
    SDL_Rect blockRect = {
        WINDOW_WIDTH / 10,
        (WINDOW_HEIGHT - totalBoxHeight) / 2,
        WINDOW_WIDTH * 8 / 10,
        totalBoxHeight
    };

    SDL_Rect boxRect = {
        blockRect.x + spacing,
        blockRect.y + spacing + labelHeight + spacing,
        blockRect.w - spacing * 2,
        boxHeight
    };

    std::vector<SDL_Rect> inputRects;
    for (int i = 0; i < size; ++i) {
        inputRects.push_back(SDL_Rect{
            boxRect.x,
            boxRect.y + i * (spacing + boxHeight),
            boxRect.w,
            boxRect.h
        });
    }

    SDL_Rect confirmRect = {
        blockRect.x + (blockRect.w - buttonWidth) / 2,
        inputRects.back().y + boxHeight + spacing + warningMessageHeight + spacing,
        buttonWidth,
        buttonHeight
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect overlayRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_RenderFillRect(renderer, &overlayRect);

    while (!done && !canceled) {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        SDL_Point mousePoint = {mx, my};
        bool mouseOverButton = SDL_PointInRect(&mousePoint, &confirmRect);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                canceled = true;
            }
            else if (e.type == SDL_TEXTINPUT) {
                if (inputs[activeInput].size() < static_cast<size_t>(MAX_CHARACTERS)) {
                    inputs[activeInput] += e.text.text;
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !inputs[activeInput].empty()) {
                    inputs[activeInput].pop_back();
                }
                else if (e.key.keysym.sym == SDLK_RETURN) {
                    if (inputs[0].empty()) {
                        warningMessage = "Name must not be empty";
                        messageTimer = SDL_GetTicks();
                    } else {
                        bool duplicate = false;
                        for (int i = 0; names[i] != nullptr; ++i) {
                            if (inputs[0] == names[i]) {
                                duplicate = true;
                                break;
                            }
                        }
                        if (duplicate) {
                            warningMessage = "This " + labelText + " already exists";
                            messageTimer = SDL_GetTicks();
                        } else {
                            done = true;
                        }
                    }
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    canceled = true;
                }
                else if (e.key.keysym.sym == SDLK_TAB) {
                    activeInput = (activeInput + 1) % size;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                SDL_Point mousePoint = { e.button.x, e.button.y };
                for (size_t i = 0; i < inputRects.size(); ++i) {
                    if (SDL_PointInRect(&mousePoint, &inputRects[i])) {
                        activeInput = static_cast<int>(i);
                        break;
                    }
                }
                if (SDL_PointInRect(&mousePoint, &confirmRect)) {
                    if (inputs[0].empty()) {
                        warningMessage = "Name must not be empty";
                        messageTimer = SDL_GetTicks();
                    } else {
                        bool duplicate = false;
                        for (int i = 0; names[i] != nullptr; ++i) {
                            if (inputs[0] == names[i]) {
                                duplicate = true;
                                break;
                            }
                        }
                        if (duplicate) {
                            warningMessage = "This " + labelText + " already exists";
                            messageTimer = SDL_GetTicks();
                        } else {
                            done = true;
                        }
                    }
                }
                if (!SDL_PointInRect(&mousePoint, &blockRect)) {
                    canceled = true;
                }
            }
        }

        // Draw block background
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderFillRect(renderer, &blockRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &blockRect);

        // Draw label
        SDL_Rect labelRect = {
            blockRect.x + (blockRect.w - labelSurf->w) / 2,
            blockRect.y + spacing,
            labelSurf->w,
            labelSurf->h
        };
        SDL_RenderCopy(renderer, labelTex, nullptr, &labelRect);

        // Draw inputs
        for (int i = 0; i < size; ++i) {
            const SDL_Rect& rect = inputRects[i];

            SDL_SetRenderDrawColor(renderer, boxColor.r, boxColor.g, boxColor.b, boxColor.a);
            SDL_RenderFillRect(renderer, &rect);

            SDL_SetRenderDrawColor(renderer,
                (i == activeInput) ? 255 : borderColor.r,
                (i == activeInput) ? 255 : borderColor.g,
                (i == activeInput) ? 0   : borderColor.b,
                255);
            SDL_RenderDrawRect(renderer, &rect);

            const std::string& content = inputs[i];
            std::string displayText = content.empty() ? placeholders[i] : content;
            SDL_Color colorToUse = content.empty() ? placeholderColor : textColor;

            SDL_Surface* textSurf = TTF_RenderText_Blended(font, displayText.c_str(), colorToUse);
            SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
            SDL_Rect textRect = {
                rect.x + 5,
                rect.y + (rect.h - textSurf->h) / 2,
                textSurf->w,
                textSurf->h
            };
            SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
            SDL_FreeSurface(textSurf);
            SDL_DestroyTexture(textTex);
        }

        // Draw button
        SDL_SetRenderDrawColor(renderer,
            mouseOverButton ? buttonHoverColor.r : buttonColor.r,
            mouseOverButton ? buttonHoverColor.g : buttonColor.g,
            mouseOverButton ? buttonHoverColor.b : buttonColor.b,
            255);
        SDL_RenderFillRect(renderer, &confirmRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &confirmRect);

        SDL_Surface* btnTextSurf = TTF_RenderText_Blended(font, "Confirm", textColor);
        SDL_Texture* btnTextTex = SDL_CreateTextureFromSurface(renderer, btnTextSurf);
        SDL_Rect btnTextRect = {
            confirmRect.x + (confirmRect.w - btnTextSurf->w) / 2,
            confirmRect.y + (confirmRect.h - btnTextSurf->h) / 2,
            btnTextSurf->w,
            btnTextSurf->h
        };
        SDL_RenderCopy(renderer, btnTextTex, nullptr, &btnTextRect);
        SDL_FreeSurface(btnTextSurf);
        SDL_DestroyTexture(btnTextTex);

        // 🔧 Show warning message if triggered
        if (messageTimer > 0) {
            Uint32 elapsed = SDL_GetTicks() - messageTimer;
            if (elapsed < messageDuration) {
                float progress = (float)elapsed / messageDuration;
                Uint8 alpha = static_cast<Uint8>(255 * (1.0f - progress)); // Fade out

                SDL_Color warningColor = { 255, 100, 100, alpha };
                SDL_Surface* warnSurf = TTF_RenderText_Blended(font, warningMessage.c_str(), warningColor);
                SDL_Texture* warnTex = SDL_CreateTextureFromSurface(renderer, warnSurf);
                SDL_SetTextureAlphaMod(warnTex, alpha); // 🔧 Important

                SDL_Rect warnRect = {
                    blockRect.x + (blockRect.w - warnSurf->w) / 2,
                    inputRects.back().y + boxHeight + spacing,
                    warnSurf->w,
                    warnSurf->h
                };
                SDL_RenderCopy(renderer, warnTex, nullptr, &warnRect);
                SDL_FreeSurface(warnSurf);
                SDL_DestroyTexture(warnTex);
            } else {
                messageTimer = 0; // Reset
            }
        }


        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    SDL_FreeSurface(labelSurf);
    SDL_DestroyTexture(labelTex);

    return { !canceled, inputs };
}


bool DeleteConfirmationPopup(SDL_Renderer* renderer, TTF_Font* font, const std::string& message, const std::string& accountName) {
    bool confirmed = false;
    bool waiting = true;

    SDL_Rect popupRect = { WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 75, 300, 150 };
    SDL_Rect yesBtn = { popupRect.x + 30, popupRect.y + 90, 100, 40 };
    SDL_Rect noBtn = { popupRect.x + 170, popupRect.y + 90, 100, 40 };

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color bgColor = { 40, 40, 40, 255 };

    // Create combined message that includes account name
    std::string fullMessage = message + "\nAccount: " + accountName;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect overlayRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_RenderFillRect(renderer, &overlayRect);

    SDL_Event e;
    while (waiting) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                waiting = false;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                if (mx < popupRect.x || mx > popupRect.x + popupRect.w ||
                    my < popupRect.y || my > popupRect.y + popupRect.h) {
                    confirmed = false;
                    waiting = false;
                }

                if (mx >= yesBtn.x && mx <= yesBtn.x + yesBtn.w &&
                    my >= yesBtn.y && my <= yesBtn.y + yesBtn.h) {
                    confirmed = true;
                    waiting = false;
                }

                if (mx >= noBtn.x && mx <= noBtn.x + noBtn.w &&
                    my >= noBtn.y && my <= noBtn.y + noBtn.h) {
                    confirmed = false;
                    waiting = false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderFillRect(renderer, &popupRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &popupRect);

        SDL_Surface* msgSurf = TTF_RenderText_Blended_Wrapped(font, fullMessage.c_str(), white, popupRect.w - 20);
        SDL_Texture* msgTex = SDL_CreateTextureFromSurface(renderer, msgSurf);
        SDL_Rect msgRect = {
            popupRect.x + (popupRect.w - msgSurf->w) / 2,
            popupRect.y + 20,
            msgSurf->w,
            msgSurf->h
        };
        SDL_RenderCopy(renderer, msgTex, nullptr, &msgRect);
        SDL_FreeSurface(msgSurf);
        SDL_DestroyTexture(msgTex);

        // Yes Button
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
        SDL_RenderFillRect(renderer, &yesBtn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &yesBtn);

        SDL_Surface* yesSurf = TTF_RenderText_Blended(font, "Yes", white);
        SDL_Texture* yesTex = SDL_CreateTextureFromSurface(renderer, yesSurf);
        SDL_Rect yesRect = {
            yesBtn.x + (yesBtn.w - yesSurf->w) / 2,
            yesBtn.y + (yesBtn.h - yesSurf->h) / 2,
            yesSurf->w,
            yesSurf->h
        };
        SDL_RenderCopy(renderer, yesTex, nullptr, &yesRect);
        SDL_FreeSurface(yesSurf);
        SDL_DestroyTexture(yesTex);

        // No Button
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &noBtn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &noBtn);

        SDL_Surface* noSurf = TTF_RenderText_Blended(font, "No", white);
        SDL_Texture* noTex = SDL_CreateTextureFromSurface(renderer, noSurf);
        SDL_Rect noRect = {
            noBtn.x + (noBtn.w - noSurf->w) / 2,
            noBtn.y + (noBtn.h - noSurf->h) / 2,
            noSurf->w,
            noSurf->h
        };
        SDL_RenderCopy(renderer, noTex, nullptr, &noRect);
        SDL_FreeSurface(noSurf);
        SDL_DestroyTexture(noTex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    return confirmed;
}


bool ServiceDetailsPopup(SDL_Renderer* renderer, TTF_Font* font, Service& service) {
    bool done = false;
    int scrollOffset = 0;
    const int blockHeight = 120;
    const int blockX = 50;
    const int blockWidth = 300;
    bool deleteService = false;
    const Uint8 colorBackground[4] = {0, 0, 0, 255};
    int paddingY = 20;
    int paddingX = 50;
    int btnHeight = 50;
    int btnWidth = 300;

    const int scrollbarWidth = 15;
    bool draggingScrollbar = false;
    int dragOffsetY = 0;

    SDL_Rect addAccountBtn = { paddingX, WINDOW_HEIGHT - paddingY - btnHeight, btnWidth, btnHeight };
    SDL_Rect deleteServiceBtn = { addAccountBtn.x, addAccountBtn.y - addAccountBtn.h - paddingY, addAccountBtn.w, addAccountBtn.h };
    SDL_Event e;
    SDL_Color white = { 255, 255, 255, 255 };

    while (!done) {
        int spacing = 20;
        int xStart = static_cast<int>(WINDOW_WIDTH * 0.1); //
        int yStart = static_cast<int>(WINDOW_HEIGHT * 0.1); //
        int yScrollArea = static_cast<int>(WINDOW_HEIGHT * 0.8); //

        SDL_Rect scrollbarTrack = {
            blockX + blockWidth + 10,  // To the right of account blocks
            yStart,
            scrollbarWidth,
            yScrollArea - yStart
        };
        int contentHeight = static_cast<int>(service.accounts.size()) * (blockHeight + spacing);
        int maxScroll = max(0, contentHeight - (yScrollArea - yStart));
        int thumbHeight = 0;
        if (contentHeight > 0) {
            thumbHeight = max(30, (yScrollArea - yStart) * (yScrollArea - yStart) / contentHeight);
        }
        int thumbY = scrollbarTrack.y + (scrollOffset * (scrollbarTrack.h - thumbHeight)) / max(1, maxScroll);
        SDL_Rect scrollbarThumb = {
            scrollbarTrack.x,
            thumbY,
            scrollbarTrack.w,
            thumbHeight
        };

        SDL_Point mousePoint;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                done = true;
            }
            else if (e.type == SDL_MOUSEWHEEL && !service.accounts.empty()) {
                scrollOffset -= e.wheel.y * 20;
                scrollOffset = max(0, min(scrollOffset, maxScroll));
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;
                mousePoint = { mx, my };

                // Start dragging scrollbar
                if (SDL_PointInRect(&mousePoint, &scrollbarThumb)) {
                    draggingScrollbar = true;
                    dragOffsetY = my - scrollbarThumb.y;
                }

                // Add account button
                if (mx >= addAccountBtn.x && mx <= addAccountBtn.x + addAccountBtn.w &&
                    my >= addAccountBtn.y && my <= addAccountBtn.y + addAccountBtn.h) {
                    const char* placeholders[2] = {"Account Name", "Password" };
                    std::vector<const char*> names = extractAccountNames(service.accounts);
                    MultiInputResult result = getMultipleTextInput(renderer, font, 2, placeholders, names.data());
                    if (result.submitted) {
                        service.accounts.push_back(Account{result.inputs[0], result.inputs[1]});
                    }
                }

                // Delete service button
                if (mx >= deleteServiceBtn.x && mx <= deleteServiceBtn.x + deleteServiceBtn.w &&
                    my >= deleteServiceBtn.y && my <= deleteServiceBtn.y + deleteServiceBtn.h) {
                    // if (DeleteConfirmationPopup(renderer, font, "Delete this service?")) {
                    //     deleteService = true;
                    //     done = true;
                    // }
                    deleteService = true;
                    done = true;
                }

                // Account delete/copy buttons
                for (size_t i = 0; i < service.accounts.size(); ++i) {
                    int y = 80 + static_cast<int>(i) * (blockHeight + spacing) - scrollOffset;
                    SDL_Rect deleteBtn = { blockX + blockWidth - 20 - 30, y + 20, 30, 30 };
                    SDL_Rect copyBtn = { blockX + 20, y + blockHeight - 20 - 30, blockWidth - 20 * 2, 30 };

                    if (mx >= deleteBtn.x && mx <= deleteBtn.x + deleteBtn.w &&
                        my >= deleteBtn.y && my <= deleteBtn.y + deleteBtn.h) {
                        if (DeleteConfirmationPopup(renderer, font, "Delete this account?", service.accounts[i].accountName)) {
                            service.accounts.erase(service.accounts.begin() + i);
                            scrollOffset = 0;
                        }
                        break;
                    }

                    if (mx >= copyBtn.x && mx <= copyBtn.x + copyBtn.w &&
                        my >= copyBtn.y && my <= copyBtn.y + copyBtn.h) {
                        SDL_SetClipboardText(service.accounts[i].password.c_str());
                        break;
                    }
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                draggingScrollbar = false;
            }
            else if (e.type == SDL_MOUSEMOTION && draggingScrollbar) {
                int my = e.motion.y;
                int newThumbY = my - dragOffsetY;
                newThumbY = max(scrollbarTrack.y, min(scrollbarTrack.y + scrollbarTrack.h - scrollbarThumb.h, newThumbY));

                float scrollRatio = float(newThumbY - scrollbarTrack.y) / float(scrollbarTrack.h - scrollbarThumb.h);
                scrollOffset = int(scrollRatio * maxScroll);
            }
        }
        // Background
        SDL_SetRenderDrawColor(renderer, colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
        SDL_RenderFillRect(renderer, nullptr);

        if (!service.accounts.empty()) {
            for (size_t i = 0; i < service.accounts.size(); ++i) {
                int y = yStart + static_cast<int>(i) * (blockHeight + spacing) - scrollOffset;
                if (y + blockHeight < 0 || y > yScrollArea) continue;

                SDL_Rect blockRect = { blockX, y, blockWidth, blockHeight };
                SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
                SDL_RenderFillRect(renderer, &blockRect);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &blockRect);

                std::string accountStr = "Account: " + service.accounts[i].accountName;
                std::string passwordStr = "Password: " + service.accounts[i].password;

                SDL_Surface* accSurf = TTF_RenderText_Blended(font, accountStr.c_str(), white);
                SDL_Texture* accTex = SDL_CreateTextureFromSurface(renderer, accSurf);
                SDL_Rect accRect = { blockRect.x + 10, blockRect.y + 10, accSurf->w, accSurf->h };
                SDL_RenderCopy(renderer, accTex, nullptr, &accRect);
                SDL_FreeSurface(accSurf);
                SDL_DestroyTexture(accTex);

                SDL_Surface* passSurf = TTF_RenderText_Blended(font, passwordStr.c_str(), white);
                SDL_Texture* passTex = SDL_CreateTextureFromSurface(renderer, passSurf);
                SDL_Rect passRect = { blockRect.x + 10, blockRect.y + 35, passSurf->w, passSurf->h };
                SDL_RenderCopy(renderer, passTex, nullptr, &passRect);
                SDL_FreeSurface(passSurf);
                SDL_DestroyTexture(passTex);
                SDL_Rect deleteBtn = { blockX + blockWidth - 20 - 30, y + 20, 30, 30 };
                SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
                SDL_RenderFillRect(renderer, &deleteBtn);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &deleteBtn);

                SDL_Surface* delSurf = TTF_RenderText_Blended(font, "X", white);
                SDL_Texture* delTex = SDL_CreateTextureFromSurface(renderer, delSurf);
                SDL_Rect delRect = {
                    deleteBtn.x + (deleteBtn.w - delSurf->w) / 2,
                    deleteBtn.y + (deleteBtn.h - delSurf->h) / 2,
                    delSurf->w,
                    delSurf->h
                };
                SDL_RenderCopy(renderer, delTex, nullptr, &delRect);
                SDL_FreeSurface(delSurf);
                SDL_DestroyTexture(delTex);

                SDL_Rect copyBtn = { blockX + 20, y + blockHeight - 20 - 30, blockWidth - 20 * 2, 30 };
                SDL_SetRenderDrawColor(renderer, 50, 150, 200, 255);
                SDL_RenderFillRect(renderer, &copyBtn);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &copyBtn);

                SDL_Surface* copySurf = TTF_RenderText_Blended(font, "Copy", white);
                SDL_Texture* copyTex = SDL_CreateTextureFromSurface(renderer, copySurf);
                SDL_Rect copyRect = {
                    copyBtn.x + (copyBtn.w - copySurf->w) / 2,
                    copyBtn.y + (copyBtn.h - copySurf->h) / 2,
                    copySurf->w,
                    copySurf->h
                };
                SDL_RenderCopy(renderer, copyTex, nullptr, &copyRect);
                SDL_FreeSurface(copySurf);
                SDL_DestroyTexture(copyTex);
            }
            // Draw rectangles to keep it beautifful :)
            SDL_Rect topEdge = { 0, 0, WINDOW_WIDTH, yStart};
            SDL_Rect bottomEdge = { 0, yScrollArea, WINDOW_WIDTH, WINDOW_HEIGHT - yScrollArea};
            // SDL_Rect leftEdge = { 0, 0, blockX, WINDOW_HEIGHT};
            // SDL_Rect rightEdge = { blockX + blockWidth, 0, blockX, WINDOW_HEIGHT };
            SDL_Rect btnsRect = {blockX, yStart, blockWidth, yScrollArea - yStart};
            SDL_SetRenderDrawColor(renderer, colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
            SDL_RenderFillRect(renderer, &topEdge);
            SDL_RenderFillRect(renderer, &bottomEdge);
            // SDL_RenderFillRect(renderer, &leftEdge);
            // SDL_RenderFillRect(renderer, &rightEdge);
            // int yLast = yStart + static_cast<int>(service.accounts.size()) * (blockHeight + spacing) - scrollOffset;
            // if (yLast >= yScrollArea) {
            //     SDL_SetRenderDrawColor(renderer,  colorBackground[0] + 50, colorBackground[1] + 50, colorBackground[2] + 50, colorBackground[3]);
            //     SDL_RenderDrawRect(renderer, &btnsRect);
            // }
            if (contentHeight > (yScrollArea - yStart)) {
                // Scrollbar background
                SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
                SDL_RenderFillRect(renderer, &scrollbarTrack);

                // Scrollbar thumb
                SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
                SDL_RenderFillRect(renderer, &scrollbarThumb);
            }
        } else {
            // Delete Service button (no accounts case)
            SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
            SDL_RenderFillRect(renderer, &deleteServiceBtn);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &deleteServiceBtn);

            SDL_Surface* delServText = TTF_RenderText_Blended(font, "Delete Service", white);
            SDL_Texture* delServTex = SDL_CreateTextureFromSurface(renderer, delServText);
            SDL_Rect delServRect = {
                deleteServiceBtn.x + (deleteServiceBtn.w - delServText->w) / 2,
                deleteServiceBtn.y + (deleteServiceBtn.h - delServText->h) / 2,
                delServText->w,
                delServText->h
            };
            SDL_RenderCopy(renderer, delServTex, nullptr, &delServRect);
            SDL_FreeSurface(delServText);
            SDL_DestroyTexture(delServTex);
        }

        // Title
        SDL_Surface* titleSurf = TTF_RenderText_Blended(font, service.label.c_str(), white);
        SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
        SDL_Rect titleRect = { 60, 30, titleSurf->w, titleSurf->h };
        SDL_RenderCopy(renderer, titleTex, nullptr, &titleRect);
        SDL_FreeSurface(titleSurf);
        SDL_DestroyTexture(titleTex);

        // Add Account button
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
        SDL_RenderFillRect(renderer, &addAccountBtn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &addAccountBtn);
        SDL_Surface* addTextSurf = TTF_RenderText_Blended(font, "Add Account", white);
        SDL_Texture* addTextTex = SDL_CreateTextureFromSurface(renderer, addTextSurf);
        SDL_Rect addTextRect = {
            addAccountBtn.x + (addAccountBtn.w - addTextSurf->w) / 2,
            addAccountBtn.y + (addAccountBtn.h - addTextSurf->h) / 2,
            addTextSurf->w,
            addTextSurf->h
        };
        SDL_RenderCopy(renderer, addTextTex, nullptr, &addTextRect);
        SDL_FreeSurface(addTextSurf);
        SDL_DestroyTexture(addTextTex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    return deleteService;
}

// used to be main(), but since I decided to use windows.h to remove console, so it needed to be changed
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    const Uint8 colorBackground[4] = {25, 25, 25, 255};

    SDL_Window* window = SDL_CreateWindow("Fixed Size Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    TTF_Font* font = TTF_OpenFont("assets/fonts/Oswald-VariableFont_wght.ttf", 16);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return 1;
    }
    std::vector<Service> services;
    loadFromFile(services, PATH_SAVE);

    auto addService = [&]() {
        const char* placeholders[1] = {"Service name"};
        std::vector<const char*> names = extractServiceNames(services);
        MultiInputResult result = getMultipleTextInput(renderer, font, 1, placeholders, names.data());
        if (result.submitted) {
            Service newService;
            newService.label = result.inputs[0];
            services.push_back(newService);
        }
    };

    Service* selectedService = nullptr;
    int scrollOffset = 0;
    const int scrollbarWidth = 15;
    bool draggingScrollbar = false;
    int dragOffsetY = 0;
    int scrollbarThumbOffsetY = 0;
    size_t lastServiceI = 0;
    bool running = true;
    SDL_Event event;
    std::string searchQuery;
    bool searchActive = false;
    int accessableButtonsCount = 0;
    std::vector<int> accessableButtons(services.size(), 0);
    Uint32 lastBlinkTime = SDL_GetTicks();
    bool showCaret = true;
    const int blinkInterval = 500; // milliseconds


    SDL_StartTextInput();
    while (running) {
        //TODO: remove from loop all constants
        int xStart = static_cast<int>(WINDOW_WIDTH * 0.1); //
        int yStart = static_cast<int>(WINDOW_HEIGHT * 0.15); //
        int yScrollArea = static_cast<int>(WINDOW_HEIGHT * 0.8); //
        int buttonWidth = WINDOW_WIDTH - 2 * xStart; //
        int buttonHeight = 50;
        int spacing = 10;
        SDL_Rect addBtnRect = { xStart + 20, yScrollArea + ((WINDOW_HEIGHT - 70 - yScrollArea) >> 1), WINDOW_WIDTH - (xStart + 20) * 2, 70 };
        SDL_Rect searchBarRect = { xStart, yStart - 80, buttonWidth, 60 };

        SDL_Rect scrollbarTrack = {
            xStart + buttonWidth + 10,  // To the right of account blocks
            yStart,
            scrollbarWidth,
            yScrollArea - yStart
        };
        int contentHeight = static_cast<int>(accessableButtonsCount) * (buttonHeight + spacing);
        int maxScroll = max(0, contentHeight - (yScrollArea - yStart));
        int thumbHeight = 0;
        if (contentHeight > 0) {
            thumbHeight = max(30, (yScrollArea - yStart) * (yScrollArea - yStart) / contentHeight);
        }
        int thumbY = scrollbarTrack.y + (scrollOffset * (scrollbarTrack.h - thumbHeight)) / max(1, maxScroll);
        SDL_Rect scrollbarThumb = {
            scrollbarTrack.x,
            thumbY,
            scrollbarTrack.w,
            thumbHeight
        };

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            else if ((event.type == SDL_MOUSEWHEEL) && (!services.empty())) {
                scrollOffset -= event.wheel.y * 20;
                int yLast = static_cast<int>(services.size()) * (buttonHeight + spacing) - scrollOffset + yStart;
                if (yLast < yScrollArea) scrollOffset = scrollOffset + yLast - yScrollArea;
                if (scrollOffset < 0) scrollOffset = 0;
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x;
                int my = event.button.y;
                SDL_Point mousePoint = { mx, my };

                if (SDL_PointInRect(&mousePoint, &searchBarRect)) {
                    searchActive = true;
                } else {
                    searchActive = false;
                }

                // Start dragging scrollbar
                if (SDL_PointInRect(&mousePoint, &scrollbarThumb)) {
                    draggingScrollbar = true;
                    dragOffsetY = my - scrollbarThumb.y;
                } else if (SDL_PointInRect(&mousePoint, &addBtnRect)) {
                    addService();
                    accessableButtons.push_back(0);
                    if (!SDL_IsTextInputActive()) {
                        SDL_StartTextInput();
                    }
                } else {
                    int notVisible = 0;
                    for (size_t i = 0; i < services.size(); ++i) {
                        if (accessableButtons[i] == 0) {
                            ++notVisible;
                            continue;
                        }
                        if (my < yStart || my > yScrollArea) continue;
                        int y = (static_cast<int>(i) - notVisible) * (buttonHeight + spacing) - scrollOffset + yStart;
                        SDL_Rect btnRect = { xStart, y, buttonWidth, buttonHeight };
                        if (mx >= btnRect.x && mx <= btnRect.x + btnRect.w &&
                            my >= btnRect.y && my <= btnRect.y + btnRect.h) {
                            // Show popup, delete service if requested
                            bool deleted = ServiceDetailsPopup(renderer, font, services[i]);
                            if (deleted) {
                                services.erase(services.begin() + i);
                                accessableButtons.erase(accessableButtons.begin() + i);
                                selectedService = nullptr;
                                scrollOffset = 0;
                            }
                            if (!SDL_IsTextInputActive()) {
                                SDL_StartTextInput();
                            }
                            break;
                        }
                    }
                }

            }

            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                draggingScrollbar = false;
            }

            else if (event.type == SDL_MOUSEMOTION && draggingScrollbar) {
                int my = event.motion.y;
                int newThumbY = my - dragOffsetY;
                newThumbY = max(scrollbarTrack.y, min(scrollbarTrack.y + scrollbarTrack.h - scrollbarThumb.h, newThumbY));

                float scrollRatio = float(newThumbY - scrollbarTrack.y) / float(scrollbarTrack.h - scrollbarThumb.h);
                scrollOffset = int(scrollRatio * maxScroll);
            }

            if (event.type == SDL_TEXTINPUT && searchActive) {
                searchQuery += event.text.text;
                scrollOffset = 0;
            } else if (event.type == SDL_KEYDOWN && searchActive) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && !searchQuery.empty()) {
                    searchQuery.pop_back();
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    searchActive = false;
                    searchQuery.clear();
                }
            }
        }



        SDL_SetRenderDrawColor(renderer, colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
        SDL_RenderClear(renderer);


        // Draw service buttons
        accessableButtonsCount = 0;
        for (size_t i = 0; i < services.size(); ++i) {
            if (!searchQuery.empty() && services[i].label.find(searchQuery) == std::string::npos) {
                accessableButtons[i] = 0;
                continue;
            }
            accessableButtons[i] = 1;

            int y = accessableButtonsCount * (buttonHeight + spacing) - scrollOffset + yStart;
            ++accessableButtonsCount;
            if (y + buttonHeight < yStart || y > yStart + (yScrollArea - yStart)) continue;
            lastServiceI = i;
            SDL_Rect btnRect = { xStart, y, buttonWidth, buttonHeight };

            SDL_SetRenderDrawColor(renderer, 70, 130, 180, 255);
            SDL_RenderFillRect(renderer, &btnRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &btnRect);

            if (!services[i].label.empty()) {
                SDL_Surface* textSurf = TTF_RenderText_Blended(font, services[i].label.c_str(), { 255, 255, 255, 255 });
                SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
                SDL_Rect textRect = {
                    btnRect.x + (btnRect.w - textSurf->w) / 2,
                    btnRect.y + (btnRect.h - textSurf->h) / 2,
                    textSurf->w,
                    textSurf->h
                };
                SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
                SDL_FreeSurface(textSurf);
                SDL_DestroyTexture(textTex);
            }

        }


        // Draw rectangles to keep it beautifful :)
        SDL_Rect topEdge = { 0, 0, WINDOW_WIDTH, yStart};
        SDL_Rect bottomEdge = { 0, yScrollArea, WINDOW_WIDTH, WINDOW_HEIGHT - yScrollArea};
        // SDL_Rect leftEdge = { 0, 0, xStart, WINDOW_HEIGHT};
        // SDL_Rect rightEdge = { xStart + buttonWidth, 0, xStart, WINDOW_HEIGHT };
        SDL_Rect btnsRect = {xStart, yStart, buttonWidth, yScrollArea - yStart};
        SDL_SetRenderDrawColor(renderer, colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
        SDL_RenderFillRect(renderer, &topEdge);
        SDL_RenderFillRect(renderer, &bottomEdge);
        // SDL_RenderFillRect(renderer, &leftEdge);
        // SDL_RenderFillRect(renderer, &rightEdge);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &btnsRect);

        // Draw scrollbar if needed
        if (contentHeight > (yScrollArea - yStart)) {
            // Scrollbar background
            SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
            SDL_RenderFillRect(renderer, &scrollbarTrack);

            // Scrollbar thumb
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
            SDL_RenderFillRect(renderer, &scrollbarThumb);
        }

        //search bar
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &searchBarRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &searchBarRect);

        // Render search text
        if (searchActive) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - lastBlinkTime >= blinkInterval) {
                showCaret = !showCaret;
                lastBlinkTime = currentTime;
            }
        } else {
            showCaret = false;
        }
        std::string displayText;
        if (!searchActive && searchQuery.empty()) {
            displayText = "Search...";
        } else if (searchActive && searchQuery.empty()) {
            displayText = " ";  // Force rendering a minimal surface for caret alignment
        } else {
            displayText = searchQuery;
        }
        SDL_Color textColor = { 255, 255, 255, 255 };
        SDL_Surface* searchSurf = TTF_RenderText_Blended(font, displayText.c_str(), textColor);
        if (searchSurf) {
            SDL_Texture* searchTex = SDL_CreateTextureFromSurface(renderer, searchSurf);
            SDL_Rect textRect = {
                searchBarRect.x + 10,
                searchBarRect.y + (searchBarRect.h - searchSurf->h) / 2,
                searchSurf->w,
                searchSurf->h
            };
            SDL_RenderCopy(renderer, searchTex, nullptr, &textRect);

            if (searchActive && showCaret) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect caretRect = {
                    textRect.x + textRect.w + 2,
                    textRect.y,
                    2,
                    textRect.h
                };
                SDL_RenderFillRect(renderer, &caretRect);
            }

            SDL_FreeSurface(searchSurf);
            SDL_DestroyTexture(searchTex);
        }


        // Draw Add Service Button
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
        SDL_RenderFillRect(renderer, &addBtnRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &addBtnRect);

        SDL_Surface* addTextSurf = TTF_RenderText_Blended(font, "Add Service", { 255, 255, 255, 255 });
        SDL_Texture* addTextTex = SDL_CreateTextureFromSurface(renderer, addTextSurf);
        SDL_Rect addTextRect = {
            addBtnRect.x + (addBtnRect.w - addTextSurf->w) / 2,
            addBtnRect.y + (addBtnRect.h - addTextSurf->h) / 2,
            addTextSurf->w,
            addTextSurf->h
        };
        SDL_RenderCopy(renderer, addTextTex, nullptr, &addTextRect);
        SDL_FreeSurface(addTextSurf);
        SDL_DestroyTexture(addTextTex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    saveToFile(services, PATH_SAVE);
    return 0;
}
