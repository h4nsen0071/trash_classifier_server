"""
Validator Manager Module
Quản lý và chạy tất cả validators
"""

import numpy as np
from typing import Dict, List, Tuple, Optional, Any

from .blur_validator import BlurValidator
from .brightness_validator import BrightnessValidator
from .blank_validator import BlankValidator


class ValidatorManager:
    """
    Manager class để quản lý và chạy các validators (image quality only).
    """
    
    def __init__(self, config):
        """
        Args:
            config: Config instance chứa validation settings
        """
        self.config = config
        self.validators = {}
        self._init_validators()
    
    def _init_validators(self) -> None:
        """Khởi tạo tất cả validators từ config"""
        cv_checks = self.config.get('validation.cv_checks', {})
        
        # Blur validator
        blur_threshold = cv_checks.get('blur_threshold', 100.0)
        self.validators['blur'] = BlurValidator(threshold=blur_threshold)
        
        # Brightness validator
        min_brightness = cv_checks.get('min_brightness', 30.0)
        max_brightness = cv_checks.get('max_brightness', 220.0)
        self.validators['brightness'] = BrightnessValidator(
            min_brightness=min_brightness,
            max_brightness=max_brightness
        )
        
        # Blank validator
        blank_std = cv_checks.get('blank_std_threshold', 10.0)
        self.validators['blank'] = BlankValidator(min_std=blank_std)
    
    def validate_image(self, image: np.ndarray) -> Dict[str, Any]:
        """
        Chạy tất cả CV validators trên ảnh.
        
        Args:
            image: Ảnh BGR từ OpenCV
            
        Returns:
            Dict chứa kết quả validation:
            {
                'valid': bool,
                'errors': List[str],
                'warnings': List[str],
                'details': Dict[str, Any]
            }
        """
        result = {
            'valid': True,
            'errors': [],
            'warnings': [],
            'details': {}
        }
        
        # Chạy các CV validators (image quality only)
        cv_validators = ['blur', 'brightness', 'blank']
        
        for name in cv_validators:
            validator = self.validators.get(name)
            if validator is None:
                continue
            
            is_valid, message, score = validator.validate(image)
            
            result['details'][name] = {
                'valid': is_valid,
                'message': message,
                'score': score
            }
            
            if not is_valid:
                result['valid'] = False
                result['errors'].append(message)
        
        return result
    
    def validate_detections(self, results) -> Dict[str, Any]:
        """
        Validate kết quả detection từ YOLO.
        
        Args:
            results: YOLO Results object
            
        Returns:
            Dict chứa kết quả validation
        """
        result = {
            'valid': True,
            'errors': [],
            'warnings': [],
            'details': {}
        }
        
        # Object count validator
        obj_validator = self.validators.get('object_count')
        if obj_validator:
            confidence_threshold = self.config.get('model.confidence_threshold', 0.25)
            is_valid, message, count = obj_validator.validate_yolo_results(
                results, confidence_threshold
            )
            
            result['details']['object_count'] = {
                'valid': is_valid,
                'message': message,
                'count': count
            }
            
            if not is_valid:
                result['valid'] = False
                result['errors'].append(message)
        
        return result
    
    def get_validator(self, name: str):
        """Lấy validator theo tên"""
        return self.validators.get(name)
    
    def list_validators(self) -> List[str]:
        """Liệt kê tên tất cả validators"""
        return list(self.validators.keys())
