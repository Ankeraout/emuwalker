#ifndef __INC_FRONTEND_FRONTEND_H__
#define __INC_FRONTEND_FRONTEND_H__

// =============================================================================
// Public functions declarations
// =============================================================================
/**
 * @brief Initializes the application front-end.
 * 
 * @returns An integer that indicates the result of the operation.
 * @retval 0 if the front-end initialization was successful.
 * @retval Any other value if an error occurred.
 */
int frontendInit(void);

/**
 * @brief This function is called everytime the core outputs a video frame and
 *        enters VBlank state.
 */
void frontendOnVBlank(void);

#endif // __INC_FRONTEND_FRONTEND_H__
