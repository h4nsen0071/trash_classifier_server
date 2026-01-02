"""
Result Validator Module
Validate và filter kết quả classification với geometric checks
"""

from typing import Dict, List, Any, Optional, Tuple


class ResultValidator:
    """
    Validator để kiểm tra và filter kết quả classification.
    Simplified cho YOLO11s-cls (single class prediction).
    """
    
    def __init__(self, config):
        """
        Args:
            config: Config instance
        """
        self.config = config
        self.min_confidence = config.get('validation.model_checks.min_confidence', 0.5)
    
    def validate(self, detections: List[Dict]) -> Dict[str, Any]:
        """
        Validate kết quả classification (simplified for single-class model).
        
        Args:
            detections: List các detection từ YOLOClassifier (chỉ có 1 item)
            
        Returns:
            Dict chứa kết quả validation:
            {
                'valid': bool,
                'errors': List[str],
                'warnings': List[str],
                'filtered_detections': List[Dict]
            }
        """
        result = {
            'valid': True,
            'errors': [],
            'warnings': [],
            'filtered_detections': []
        }
        
        # Check: không có detection nào
        if len(detections) == 0:
            result['valid'] = False
            result['errors'].append("Không tìm thấy rác trong ảnh")
            return result
        
        # Filter by confidence
        filtered = [d for d in detections if d['confidence'] >= self.min_confidence]
        result['filtered_detections'] = filtered
        
        # Check: không có detection nào đạt confidence threshold
        if len(filtered) == 0:
            result['valid'] = False
            result['errors'].append(
                f"Không có detection nào đạt confidence >= {self.min_confidence}"
            )
            return result
        
        return result
    
    def get_best_detection(self, detections: List[Dict]) -> Optional[Dict]:
        """
        Lấy detection tốt nhất (classification chỉ có 1 result).
        
        Args:
            detections: List các detection
            
        Returns:
            Detection đầu tiên (top-1) hoặc None
        """
        if not detections:
            return None
        return detections[0]  # Classification only returns top-1
    
    def should_retry(self, validation_result: Dict) -> bool:
        """
        Kiểm tra xem có nên yêu cầu retry không.
        
        Args:
            validation_result: Kết quả từ validate()
            
        Returns:
            True nếu nên retry (ảnh không hợp lệ nhưng có thể fix được)
        """
        errors = validation_result.get('errors', [])
        
        # Các lỗi có thể retry
        retryable_errors = [
            'Không tìm thấy rác',
            'confidence',
            'mờ',
            'sáng'
        ]
        
        for error in errors:
            for retryable in retryable_errors:
                if retryable in error:
                    return True
        
        return False
