"""
Response Builder Module
Xây dựng standardized API responses
"""

from typing import Dict, List, Any, Optional
from flask import jsonify


class ResponseBuilder:
    """
    Builder class để tạo standardized API responses.
    """
    
    @staticmethod
    def success(data: Dict[str, Any], 
                message: str = "Success") -> Dict[str, Any]:
        """
        Build success response.
        
        Args:
            data: Response data
            message: Success message
            
        Returns:
            Standardized success response dict
        """
        return {
            'success': True,
            'message': message,
            'data': data
        }
    
    @staticmethod
    def error(message: str,
             code: str = "ERROR",
             details: Optional[Dict] = None) -> Dict[str, Any]:
        """
        Build error response.
        
        Args:
            message: Error message
            code: Error code
            details: Additional error details
            
        Returns:
            Standardized error response dict
        """
        response = {
            'success': False,
            'error': {
                'code': code,
                'message': message
            }
        }
        
        if details:
            response['error']['details'] = details
        
        return response
    
    @staticmethod
    def classification_result(class_name: str,
                             bin_number: int,
                             confidence: float,
                             detections: List[Dict],
                             warnings: Optional[List[str]] = None) -> Dict[str, Any]:
        """
        Build classification result response (simplified for classification model).
        
        Args:
            class_name: Tên class được phân loại
            bin_number: Số thùng rác
            confidence: Độ tin cậy
            detections: Danh sách các detections (không dùng, giữ compatibility)
            warnings: Các cảnh báo (nếu có)
            
        Returns:
            Classification response dict
        """
        data = {
            'class': class_name,
            'bin': bin_number,
            'confidence': round(confidence, 4)
        }
        
        if warnings:
            data['warnings'] = warnings
        
        return ResponseBuilder.success(data, "Classification successful")
    
    @staticmethod
    def validation_error(errors: List[str],
                        validation_details: Optional[Dict] = None) -> Dict[str, Any]:
        """
        Build validation error response.
        
        Args:
            errors: List các validation errors
            validation_details: Chi tiết validation
            
        Returns:
            Validation error response dict
        """
        return ResponseBuilder.error(
            message="Validation failed",
            code="VALIDATION_ERROR",
            details={
                'errors': errors,
                'validation': validation_details
            }
        )
    
    @staticmethod
    def no_detection_error(validation_details: Optional[Dict] = None) -> Dict[str, Any]:
        """
        Build no detection error response.
        
        Args:
            validation_details: Chi tiết validation
            
        Returns:
            No detection error response dict
        """
        return ResponseBuilder.error(
            message="Không tìm thấy rác trong ảnh",
            code="NO_DETECTION",
            details={
                'validation': validation_details,
                'suggestion': "Hãy đặt rác vào vùng nhìn của camera và thử lại"
            }
        )
    
    @staticmethod
    def health_check(status: str = "healthy",
                    model_loaded: bool = True,
                    version: str = "1.0.0") -> Dict[str, Any]:
        """
        Build health check response.
        
        Args:
            status: Server status
            model_loaded: Model có được load không
            version: Server version
            
        Returns:
            Health check response dict
        """
        return {
            'status': status,
            'model_loaded': model_loaded,
            'version': version
        }


# Shortcut functions
def json_success(data: Dict, message: str = "Success"):
    """Return Flask JSON response for success"""
    return jsonify(ResponseBuilder.success(data, message))


def json_error(message: str, code: str = "ERROR", status_code: int = 400, details: Optional[Dict] = None):
    """Return Flask JSON response for error"""
    response = jsonify(ResponseBuilder.error(message, code, details))
    response.status_code = status_code
    return response


def json_classification(class_name: str, bin_number: int, confidence: float, 
                       detections: List[Dict], warnings: Optional[List[str]] = None):
    """Return Flask JSON response for classification"""
    return jsonify(ResponseBuilder.classification_result(
        class_name, bin_number, confidence, detections, warnings
    ))
