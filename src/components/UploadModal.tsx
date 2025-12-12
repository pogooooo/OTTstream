// components/UploadModal.tsx

import React, { useState } from 'react';
import styled from 'styled-components';
import { useSelector } from 'react-redux';
import type {RootState} from '../Store';
import { uploadVideo } from '../utils/api.tsx';

interface UploadModalProps {
    isOpen: boolean;
    onClose: () => void;
    onUploadSuccess: () => void;
}

const UploadModal: React.FC<UploadModalProps> = ({ isOpen, onClose, onUploadSuccess }) => {
    const [file, setFile] = useState<File | null>(null);
    const [title, setTitle] = useState('');
    const [isLoading, setIsLoading] = useState(false);
    const [message, setMessage] = useState('');

    const token = useSelector((state: RootState) => state.user.token);

    const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        if (e.target.files && e.target.files.length > 0) {
            setFile(e.target.files[0]);
            setMessage('');
        }
    };

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        if (!file || !title) {
            setMessage('제목과 MP4 파일을 모두 선택해주세요.');
            return;
        }

        setIsLoading(true);
        setMessage('파일 업로드 중...');

        if (file.type !== 'video/mp4') {
            setMessage('오직 MP4 파일만 업로드할 수 있습니다.');
            setFile(null);
            setIsLoading(false);
            return;
        }

        if (!token) {
            setMessage('로그인 상태가 아닙니다. 재로그인해주세요.');
            setIsLoading(false);
            return;
        }

        const result = await uploadVideo(file, title, token);

        setIsLoading(false);
        if (result.success) {
            setMessage('업로드 성공!');
            onUploadSuccess();
            setTimeout(onClose, 1000);
        } else {
            setMessage(result.message || '업로드 중 알 수 없는 오류 발생');
        }
    };

    if (!isOpen) return null;

    return (
        <ModalOverlay onClick={onClose}>
            <ModalContent onClick={e => e.stopPropagation()}>
                <CloseButton onClick={onClose}>&times;</CloseButton>
                <h2>새 영상 업로드</h2>
                <UploadForm onSubmit={handleSubmit}>
                    <Input
                        type="text"
                        placeholder="영상 제목을 입력하세요"
                        value={title}
                        onChange={(e) => setTitle(e.target.value)}
                        required
                        disabled={isLoading}
                    />
                    <InputFile type="file" accept="video/mp4" onChange={handleFileChange} disabled={isLoading} />
                    {file && <p>선택된 파일: {file.name}</p>}
                    <Button type="submit" disabled={isLoading}>
                        {isLoading ? '업로드 중...' : '업로드 시작'}
                    </Button>
                </UploadForm>
                <p style={{ color: message.includes('성공') ? 'green' : 'red', marginTop: '10px' }}>{message}</p>
            </ModalContent>
        </ModalOverlay>
    );
};

const ModalOverlay = styled.div`
    position: fixed;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background: rgba(0, 0, 0, 0.7);
    display: flex;
    justify-content: center;
    align-items: center;
    z-index: 2000;
`;

const ModalContent = styled.div`
    background: white;
    padding: 30px;
    border-radius: 10px;
    width: 450px;
    max-width: 90%;
    position: relative;
`;

const CloseButton = styled.button`
    position: absolute;
    top: 10px;
    right: 15px;
    background: none;
    border: none;
    font-size: 24px;
    cursor: pointer;
`;

const UploadForm = styled.form`
    display: flex;
    flex-direction: column;
    gap: 15px;
`;

const Input = styled.input`
    padding: 10px;
    border: 1px solid #ccc;
    border-radius: 5px;
`;

const InputFile = styled.input`
    padding: 10px 0;
`;

const Button = styled.button`
    padding: 12px;
    background-color: #e50914;
    color: white;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    font-weight: bold;
    &:disabled {
        background-color: #a0a0a0;
        cursor: not-allowed;
    }
`;


export default UploadModal;
