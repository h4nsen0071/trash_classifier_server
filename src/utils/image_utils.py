"""
Image Utilities Module
Xử lý và chuyển đổi ảnh
"""

import cv2
import numpy as np
import base64
from typing import Tuple, Optional, Union
from io import BytesIO
from PIL import Image


class ImageUtils:
    """
    Utility class cho các thao tác xử lý ảnh.
    """
    
    @staticmethod
    def decode_base64(base64_string: str) -> Optional[np.ndarray]:
        """
        Decode base64 string thành OpenCV image.
        
        Args:
            base64_string: Base64 encoded image string
            
        Returns:
            Ảnh BGR từ OpenCV hoặc None nếu lỗi
        """
        try:
            # Remove header nếu có (data:image/jpeg;base64,)
            if ',' in base64_string:
                base64_string = base64_string.split(',')[1]
            
            # Decode base64
            img_bytes = base64.b64decode(base64_string)
            
            # Convert sang numpy array
            nparr = np.frombuffer(img_bytes, np.uint8)
            
            # Decode image
            image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            
            return image
            
        except Exception as e:
            print(f"Error decoding base64: {e}")
            return None
    
    @staticmethod
    def encode_base64(image: np.ndarray, format: str = 'jpeg') -> Optional[str]:
        """
        Encode OpenCV image thành base64 string.
        
        Args:
            image: Ảnh BGR từ OpenCV
            format: Output format ('jpeg' hoặc 'png')
            
        Returns:
            Base64 encoded string hoặc None nếu lỗi
        """
        try:
            # Encode image
            if format.lower() == 'png':
                success, buffer = cv2.imencode('.png', image)
            else:
                success, buffer = cv2.imencode('.jpg', image, [cv2.IMWRITE_JPEG_QUALITY, 85])
            
            if not success:
                return None
            
            # Convert sang base64
            base64_string = base64.b64encode(buffer).decode('utf-8')
            
            return base64_string
            
        except Exception as e:
            print(f"Error encoding base64: {e}")
            return None
    
    @staticmethod
    def read_file_bytes(file_bytes: bytes) -> Optional[np.ndarray]:
        """
        Đọc ảnh từ file bytes.
        
        Args:
            file_bytes: Raw bytes của file ảnh
            
        Returns:
            Ảnh BGR từ OpenCV hoặc None nếu lỗi
        """
        try:
            nparr = np.frombuffer(file_bytes, np.uint8)
            image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            return image
        except Exception as e:
            print(f"Error reading file bytes: {e}")
            return None
    
    @staticmethod
    def resize(image: np.ndarray, 
               max_size: int = 640,
               keep_aspect: bool = True) -> np.ndarray:
        """
        Resize ảnh với optional aspect ratio preservation.
        
        Args:
            image: Ảnh BGR
            max_size: Kích thước tối đa (width hoặc height)
            keep_aspect: Giữ tỉ lệ hay không
            
        Returns:
            Ảnh đã resize
        """
        h, w = image.shape[:2]
        
        if keep_aspect:
            if h > w:
                new_h = max_size
                new_w = int(w * max_size / h)
            else:
                new_w = max_size
                new_h = int(h * max_size / w)
        else:
            new_w = max_size
            new_h = max_size
        
        return cv2.resize(image, (new_w, new_h), interpolation=cv2.INTER_LINEAR)
    
    @staticmethod
    def draw_classification_result(image: np.ndarray, 
                                   class_name: str,
                                   confidence: float,
                                   bin_num: int) -> np.ndarray:
        """
        Vẽ classification result lên ảnh (simplified - không có bounding box).
        
        Args:
            image: Ảnh BGR
            class_name: Tên class
            confidence: Confidence score
            bin_num: Số bin
            
        Returns:
            Ảnh với text annotation
        """
        result = image.copy()
        
        # Colors cho từng class
        colors = {
            'paper': (0, 255, 0),      # Green
            'plastic': (0, 165, 255),   # Orange
            'glass': (255, 0, 0)        # Blue
        }
        color = colors.get(class_name, (128, 128, 128))
        
        # Label text
        label = f"{class_name.upper()}: {confidence:.1%} -> Bin {bin_num}"
        
        # Draw on top-left corner with background
        font = cv2.FONT_HERSHEY_SIMPLEX
        font_scale = 1.0
        thickness = 2
        
        (text_w, text_h), baseline = cv2.getTextSize(label, font, font_scale, thickness)
        
        # Background rectangle
        cv2.rectangle(result, (10, 10), (20 + text_w, 20 + text_h), color, -1)
        
        # Text
        cv2.putText(result, label, (15, 15 + text_h), font, font_scale, (255, 255, 255), thickness)
        
        return result
    
    @staticmethod
    def validate_image_size(image: np.ndarray,
                           min_size: int = 32,
                           max_size: int = 4096) -> Tuple[bool, str]:
        """
        Validate kích thước ảnh.
        
        Args:
            image: Ảnh
            min_size: Kích thước tối thiểu
            max_size: Kích thước tối đa
            
        Returns:
            Tuple[bool, str]: (is_valid, message)
        """
        if image is None:
            return False, "Invalid image (None)"
        
        h, w = image.shape[:2]
        
        if h < min_size or w < min_size:
            return False, f"Ảnh quá nhỏ ({w}x{h}), tối thiểu {min_size}x{min_size}"
        
        if h > max_size or w > max_size:
            return False, f"Ảnh quá lớn ({w}x{h}), tối đa {max_size}x{max_size}"
        
        return True, f"Kích thước OK ({w}x{h})"
