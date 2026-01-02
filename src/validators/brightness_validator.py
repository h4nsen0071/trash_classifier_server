"""
Brightness Validator Module
Kiểm tra độ sáng của ảnh
"""

import cv2
import numpy as np
from typing import Tuple


class BrightnessValidator:
    """
    Validator để kiểm tra ảnh có quá tối hoặc quá sáng.
    Sử dụng mean brightness trong không gian màu HSV.
    """
    
    def __init__(self, min_brightness: float = 30.0, max_brightness: float = 220.0):
        """
        Args:
            min_brightness: Ngưỡng tối thiểu (0-255)
            max_brightness: Ngưỡng tối đa (0-255)
        """
        self.min_brightness = min_brightness
        self.max_brightness = max_brightness
        self.name = "brightness_validator"
    
    def validate(self, image: np.ndarray) -> Tuple[bool, str, float]:
        """
        Kiểm tra độ sáng của ảnh.
        
        Args:
            image: Ảnh BGR từ OpenCV
            
        Returns:
            Tuple[bool, str, float]: (is_valid, message, brightness_value)
        """
        # Convert sang HSV và lấy channel V (Value/Brightness)
        if len(image.shape) == 3:
            hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            brightness = np.mean(hsv[:, :, 2])
        else:
            brightness = np.mean(image)
        
        # Kiểm tra ngưỡng
        if brightness < self.min_brightness:
            return False, f"Ảnh quá tối (brightness: {brightness:.2f} < {self.min_brightness})", brightness
        
        if brightness > self.max_brightness:
            return False, f"Ảnh quá sáng (brightness: {brightness:.2f} > {self.max_brightness})", brightness
        
        return True, f"Độ sáng OK (brightness: {brightness:.2f})", brightness
    
    def __repr__(self) -> str:
        return f"BrightnessValidator(min={self.min_brightness}, max={self.max_brightness})"
