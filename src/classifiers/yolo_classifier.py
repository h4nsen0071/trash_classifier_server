"""
YOLO Classifier Module
Thực hiện inference và post-processing
"""

import numpy as np
from typing import Dict, List, Any, Optional
from ultralytics import YOLO


class YOLOClassifier:
    """
    Wrapper class cho YOLO model để thực hiện trash classification.
    """
    
    def __init__(self, model: YOLO, config):
        """
        Args:
            model: YOLO model instance
            config: Config instance
        """
        self.model = model
        self.config = config
        self.classes = config.get('classes', ['paper', 'plastic', 'glass'])
        self.bin_mapping = config.get('bin_mapping', {})
        self.confidence_threshold = config.get('model.confidence_threshold', 0.25)
    
    def predict(self, image: np.ndarray) -> Dict[str, Any]:
        """
        Thực hiện prediction trên ảnh (YOLO11s-cls classification).
        
        Args:
            image: Ảnh BGR từ OpenCV
            
        Returns:
            Dict chứa kết quả prediction:
            {
                'success': bool,
                'detections': List[Dict],
                'raw_results': YOLO Results
            }
        """
        try:
            # Chạy inference (classification model)
            results = self.model(
                image,
                verbose=False
            )
            
            # Parse classification results
            detections = self._parse_classification_results(results)
            
            return {
                'success': True,
                'detections': detections,
                'raw_results': results
            }
            
        except Exception as e:
            return {
                'success': False,
                'error': str(e),
                'detections': [],
                'raw_results': None
            }
    
    def _parse_classification_results(self, results) -> List[Dict[str, Any]]:
        """
        Parse YOLO11s-cls classification results.
        
        Args:
            results: YOLO Results object (classification)
            
        Returns:
            List với 1 detection dict (top-1 prediction)
        """
        detections = []
        
        try:
            probs = results[0].probs
            if probs is None:
                return detections
            
            # Lấy top-1 prediction
            top1_idx = int(probs.top1)  # Class ID
            top1_conf = float(probs.top1conf)  # Confidence
            
            # Lấy class name từ model.names (alphabet sorted)
            class_name = self.model.names[top1_idx]
            
            # Chỉ trả về nếu confidence >= threshold
            if top1_conf >= self.confidence_threshold:
                detection = {
                    'class_id': top1_idx,
                    'class_name': class_name,
                    'confidence': top1_conf,
                    'bin': int(self._get_bin(class_name))
                }
                detections.append(detection)
            
        except Exception as e:
            print(f"Error parsing classification results: {e}")
        
        return detections
    
    def _get_class_name(self, class_id: int) -> str:
        """Lấy tên class từ ID"""
        if 0 <= class_id < len(self.classes):
            return self.classes[class_id]
        return f"unknown_{class_id}"
    
    def _get_bin(self, class_name: str) -> int:
        """Lấy bin number từ class name"""
        return self.bin_mapping.get(class_name, 0)
    
    def get_primary_detection(self, detections: List[Dict]) -> Optional[Dict]:
        """
        Lấy detection chính (confidence cao nhất).
        
        Args:
            detections: List các detection
            
        Returns:
            Detection với confidence cao nhất hoặc None
        """
        if not detections:
            return None
        return detections[0]  # Already sorted by confidence
    
    def get_detection_summary(self, detections: List[Dict]) -> Dict[str, Any]:
        """
        Tạo summary cho các detections.
        
        Args:
            detections: List các detection
            
        Returns:
            Dict summary
        """
        if not detections:
            return {
                'total_objects': 0,
                'primary_class': None,
                'primary_bin': 0,
                'classes_found': []
            }
        
        primary = self.get_primary_detection(detections)
        classes_found = list(set(d['class_name'] for d in detections))
        
        return {
            'total_objects': len(detections),
            'primary_class': primary['class_name'],
            'primary_confidence': primary['confidence'],
            'primary_bin': primary['bin'],
            'classes_found': classes_found
        }
