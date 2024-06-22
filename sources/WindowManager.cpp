#include "WindowManager.h"
#include "Mesh.h"

const double WindowManager::MIN_TARGET_FPS = 5.0;
const double WindowManager::MAX_TARGET_FPS = 1000.0;

void WindowManager::init(int width, int height, double theTargetFps, bool theVerboseSetting) {
    setupOpenGL(width, height);

    setTargetFps(theTargetFps);

    totalFrameCount = 0;
    frameCount = 0;
    currentFps = 0.0;
    sleepDuration = 0.0;
    frameStartTime = glfwGetTime();
    frameEndTime = frameStartTime + 1;
    frameDuration = 1;
    lastReportTime = frameStartTime;
    reportInterval = 1.0f;
    windowTitle = "NONE";
    verbose = theVerboseSetting;
}

void WindowManager::setupOpenGL(int width, int height) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    window = glfwCreateWindow(width, height, "Zadatak X", nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();

    // dohvati sve dostupne OpenGL funkcije
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD");
        exit(-1);
    }
    fprintf(stdout, "OpenGL %s\n", glGetString(GL_VERSION));
    GLCheckError();
}

WindowManager::WindowManager(int width, int height, int theTargetFps) { init(width, height, theTargetFps, false); }

WindowManager::WindowManager(int width, int height, int theTargetFps, double theReportInterval) {
    init(width, height, theTargetFps, true);
    setReportInterval(theReportInterval);
}

WindowManager::WindowManager(int width, int height, int theTargetFps, float theReportInterval,
                             std::string theWindowTitle) {
    init(width, height, theTargetFps,
         true); // If you specify a window title it's safe to say you want the FPS to update there ;)
    setReportInterval(theReportInterval);
    windowTitle = theWindowTitle;
}

bool WindowManager::getVerbose() { return verbose; }

void WindowManager::setVerbose(bool theVerboseValue) { verbose = theVerboseValue; }

int WindowManager::getTargetFps() { return targetFps; }

void WindowManager::setTargetFps(int theFpsLimit) {
    // Make at least some attempt to sanitise the target FPS...
    if (theFpsLimit < MIN_TARGET_FPS) {
        theFpsLimit = MIN_TARGET_FPS;
        std::cout << "Limiting FPS rate to legal minimum of " << MIN_TARGET_FPS << " frames per second." << std::endl;
    }
    if (theFpsLimit > MAX_TARGET_FPS) {
        theFpsLimit = MAX_TARGET_FPS;
        std::cout << "Limiting FPS rate to legal maximum of " << MAX_TARGET_FPS << " frames per second." << std::endl;
    }

    // ...then set it and calculate the target duration of each frame at this framerate
    targetFps = theFpsLimit;
    targetFrameDuration = 1.0 / targetFps;
}

double WindowManager::getFrameDuration() { return frameDuration; }

void WindowManager::setReportInterval(float theReportInterval) {
    // Ensure the time interval between FPS checks is sane (low cap = 0.1s, high-cap = 10.0s)
    // Negative numbers are invalid, 10 fps checks per second at most, 1 every 10 secs at least.
    if (theReportInterval < 0.1) {
        theReportInterval = 0.1;
    }
    if (theReportInterval > 10.0) {
        theReportInterval = 10.0;
    }
    reportInterval = theReportInterval;
}

double WindowManager::LimitFPS(bool shouldSleep) {
    // increase total number of drawnFrames
    totalFrameCount++;

    // Get the current time
    frameEndTime = glfwGetTime();

    // Calculate how long it's been since the frameStartTime was set (at the end of this method)
    frameDuration = frameEndTime - frameStartTime;

    if (reportInterval != 0.0f) {

        // Calculate and display the FPS every specified time interval
        if ((frameEndTime - lastReportTime) > reportInterval) {
            // Update the last report time to be now
            lastReportTime = frameEndTime;

            // Calculate the FPS as the number of frames divided by the interval in seconds
            currentFps = (double)frameCount / reportInterval;

            // Reset the frame counter to 1 (and not zero - which would make our FPS values off)
            frameCount = 1;

            if (verbose) {
                // std::cout << "FPS: " << currentFps << std::endl;

                // If the user specified a window title to append the FPS value to...
                if (windowTitle != "NONE") {
                    // Convert the fps value into a string using an output stringstream
                    std::ostringstream stream;
                    stream << currentFps;
                    std::string fpsString = stream.str();

                    // Append the FPS value to the window title details
                    std::string tempWindowTitle = windowTitle + " | FPS: " + fpsString;

                    // Convert the new window title to a c_str and set it
                    const char *pszConstString = tempWindowTitle.c_str();
                    glfwSetWindowTitle(window, pszConstString);
                    // printf("%s\n", pszConstString);
                }

            } // End of if verbose section

        } else { // FPS calculation time interval hasn't elapsed yet? Simply increment the FPS frame counter
            ++frameCount;
        }

    } // End of if we specified a report interval section

    // Calculate how long we should sleep for to stick to our target frame rate
    sleepDuration = targetFrameDuration - frameDuration;

    // If we're running faster than our target duration, sleep until we catch up!
    if (shouldSleep && sleepDuration > 0.0) {
        std::this_thread::sleep_for(std::chrono::microseconds((int)(1000000 * (targetFrameDuration - frameDuration))));
    }

    // Reset the frame start time to be now - this means we only need put a single call into the main loop
    frameStartTime = glfwGetTime();

    // Pass back our total frame duration (including any sleep and the time it took to run this function) to be used as
    // our deltaTime value
    return frameDuration + (frameStartTime - frameEndTime);
}

int WindowManager::getFrameCount() { return totalFrameCount; }

void WindowManager::PollEvents() { glfwPollEvents(); }