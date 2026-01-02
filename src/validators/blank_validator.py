"""
Blank Image Validator Module
Kiểm tra ảnh có bị trống/đơn màu không
"""

import cv2
import numpy as np
from typing import Tuple


class BlankValidator:
    """
    Validator để kiểm tra ảnh có bị trống (blank/solid color).
    Sử dụng standard deviation của pixel values.
    """
    
    def __init__(self, min_std: float = 10.0):
        """
        Args:
            min_std: Ngưỡng standard deviation tối thiểu.
                    Ảnh có std < min_std được coi là trống/đơn màu.
        """
        self.min_std = min_std
        self.name = "blank_validator"
    
    def validate(self, image: np.ndarray) -> Tuple[bool, str, float]:
        """
        Kiểm tra ảnh có bị trống không.
        
        Args:
            image: Ảnh BGR từ OpenCV
            
        Returns:
            Tuple[bool, str, float]: (is_valid, message, std_value)
        """
        # Convert sang grayscale
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image
        
        # Tính standard deviation
        std = np.std(gray)
        
        # Kiểm tra ngưỡng
        if std < self.min_std:
            return False, f"Ảnh trống/đơn màu (std: {std:.2f} < {self.min_std})", std
        
        return True, f"Ảnh OK (std: {std:.2f})", std
    
    def __repr__(self) -> str:
        return f"BlankValidator(min_std={self.min_std})"
