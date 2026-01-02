"""
Blur Validator Module
Kiểm tra độ mờ của ảnh sử dụng Laplacian variance
"""

import cv2
import numpy as np
from typing import Tuple


class BlurValidator:
    """
    Validator để kiểm tra ảnh có bị mờ hay không.
    Sử dụng Laplacian variance method.
    """
    
    def __init__(self, threshold: float = 100.0):
        """
        Args:
            threshold: Ngưỡng blur. Ảnh có variance < threshold được coi là mờ.
                      Giá trị thấp = ảnh mờ, giá trị cao = ảnh rõ nét.
        """
        self.threshold = threshold
        self.name = "blur_validator"
    
    def validate(self, image: np.ndarray) -> Tuple[bool, str, float]:
        """
        Kiểm tra độ mờ của ảnh.
        
        Args:
            image: Ảnh BGR từ OpenCV
            
        Returns:
            Tuple[bool, str, float]: (is_valid, message, variance_score)
        """
        # Convert sang grayscale
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image
        
        # Tính Laplacian variance
        laplacian = cv2.Laplacian(gray, cv2.CV_64F)
        variance = laplacian.var()
        
        # Kiểm tra ngưỡng
        if variance < self.threshold:
            return False, f"Ảnh bị mờ (score: {variance:.2f} < {self.threshold})", variance
        
        return True, f"Độ nét OK (score: {variance:.2f})", variance
    
    def __repr__(self) -> str:
        return f"BlurValidator(threshold={self.threshold})"
