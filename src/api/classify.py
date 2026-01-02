"""
Classify API Blueprint
Endpoint chính để phân loại rác
"""

import time
from flask import Blueprint, request, jsonify

from ..config import Config
from ..model_loader import ModelLoader
from ..validators import ValidatorManager
from ..classifiers import YOLOClassifier, ResultValidator
from ..utils import ImageUtils, ResponseBuilder
from ..utils.logger import logger


# Create Blueprint
classify_bp = Blueprint('classify', __name__)


@classify_bp.route('/classify', methods=['POST'])
def classify():
    """
    Phân loại rác từ ảnh.
    
    Input: JSON với base64 image hoặc form-data với file
    Output: JSON với class, bin number, confidence
    """
    start_time = time.time()
    
    # Log request
    logger.log_request('/classify', 'POST', request.remote_addr)
    
    # Load dependencies
    config = Config()
    model_loader = ModelLoader(config)
    validator_manager = ValidatorManager(config)
    
    # Get image from request
    image = _get_image_from_request(request)
    
    if image is None:
        return jsonify(ResponseBuilder.error(
            message="Không thể đọc ảnh từ request",
            code="INVALID_IMAGE"
        )), 400
    
    # Validate image size
    is_valid, msg = ImageUtils.validate_image_size(image)
    if not is_valid:
        return jsonify(ResponseBuilder.error(
            message=msg,
            code="INVALID_IMAGE_SIZE"
        )), 400
    
    # Validate image quality (CV checks)
    validation_result = validator_manager.validate_image(image)
    
    if not validation_result['valid']:
        logger.log_validation_error(validation_result['errors'])
        return jsonify(ResponseBuilder.validation_error(
            errors=validation_result['errors'],
            validation_details=validation_result['details']
        )), 400
    
    # Run classification
    model = model_loader.get_model()
    classifier = YOLOClassifier(model, config)
    result_validator = ResultValidator(config)
    
    prediction = classifier.predict(image)
    
    if not prediction['success']:
        return jsonify(ResponseBuilder.error(
            message=f"Model error: {prediction.get('error', 'Unknown')}",
            code="MODEL_ERROR"
        )), 500
    
    # Validate detection results (confidence-based only)
    detection_validation = result_validator.validate(prediction['detections'])
    
    # Combine warnings
    all_warnings = validation_result.get('warnings', []) + detection_validation.get('warnings', [])
    
    if not detection_validation['valid']:
        # No valid detections
        return jsonify(ResponseBuilder.no_detection_error(
            validation_details={
                'image_validation': validation_result['details'],
                'detection_validation': detection_validation
            }
        )), 400
    
    # Get best detection
    best_detection = result_validator.get_best_detection(prediction['detections'])
    
    # Calculate duration
    duration_ms = (time.time() - start_time) * 1000
    
    # Log result
    logger.log_classification({
        'class': best_detection['class_name'],
        'confidence': best_detection['confidence'],
        'bin': best_detection['bin']
    }, duration_ms)
    
    # Build response
    response = ResponseBuilder.classification_result(
        class_name=best_detection['class_name'],
        bin_number=best_detection['bin'],
        confidence=best_detection['confidence'],
        detections=detection_validation['filtered_detections'],
        warnings=all_warnings if all_warnings else None
    )
    
    # Add processing time
    response['data']['processing_time_ms'] = round(duration_ms, 2)
    
    return jsonify(response), 200


def _get_image_from_request(req):
    """
    Extract image từ request (JSON hoặc form-data).
    
    Args:
        req: Flask request object
        
    Returns:
        OpenCV image hoặc None
    """
    try:
        # Check JSON request
        if req.is_json:
            data = req.get_json()
            if 'image' in data:
                return ImageUtils.decode_base64(data['image'])
        
        # Check form-data
        if 'image' in req.files:
            file = req.files['image']
            return ImageUtils.read_file_bytes(file.read())
        
        # Check raw data
        if req.data:
            return ImageUtils.read_file_bytes(req.data)
        
    except Exception as e:
        logger.error(f"Error extracting image: {e}")
    
    return None


@classify_bp.route('/classify/debug', methods=['POST'])
def classify_debug():
    """
    Debug endpoint - trả về annotated image.
    Chỉ enable trong development mode.
    """
    config = Config()
    
    if not config.get('server.debug', False):
        return jsonify(ResponseBuilder.error(
            message="Debug endpoint disabled in production",
            code="DISABLED"
        )), 403
    
    # Get image
    image = _get_image_from_request(request)
    if image is None:
        return jsonify(ResponseBuilder.error(
            message="Không thể đọc ảnh",
            code="INVALID_IMAGE"
        )), 400
    
    # Run classification
    model_loader = ModelLoader(config)
    classifier = YOLOClassifier(model_loader.get_model(), config)
    result_validator = ResultValidator(config)
    
    prediction = classifier.predict(image)
    
    if not prediction['success'] or len(prediction['detections']) == 0:
        return jsonify(ResponseBuilder.error(
            message="No classification result",
            code="NO_DETECTION"
        )), 400
    
    # Get best detection
    best = result_validator.get_best_detection(prediction['detections'])
    
    # Draw classification result
    annotated = ImageUtils.draw_classification_result(
        image, 
        best['class_name'],
        best['confidence'],
        best['bin']
    )
    
    # Encode result
    result_base64 = ImageUtils.encode_base64(annotated)
    
    return jsonify(ResponseBuilder.success({
        'annotated_image': result_base64,
        'classification': {
            'class': best['class_name'],
            'confidence': best['confidence'],
            'bin': best['bin']
        }
    })), 200
