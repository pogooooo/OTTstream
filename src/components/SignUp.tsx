import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import styled from 'styled-components';
import { signUpApi } from '../utils/api.tsx';

const SignUp = () => {
    const [nickname, setNickname] = useState('');
    const [password, setPassword] = useState('');
    const [message, setMessage] = useState('');
    const navigate = useNavigate();

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        setMessage('회원가입 시도 중...');

        const result = await signUpApi({ nickname, password });

        if (result.success) {
            setMessage('회원가입 성공! 로그인 페이지로 이동합니다.');
            setTimeout(() => navigate('/signIn'), 1500);
        } else {
            setMessage(result.message);
        }
    };

    return (
        <AuthWrapper>
            <Form onSubmit={handleSubmit}>
                <h2>회원가입</h2>
                <Input
                    type="text"
                    placeholder="사용자 닉네임"
                    value={nickname}
                    onChange={(e) => setNickname(e.target.value)}
                    required
                />
                <Input
                    type="password"
                    placeholder="비밀번호"
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    required
                />
                <Button type="submit">가입하기</Button>
                <p style={{ color: message.includes('성공') ? 'green' : 'red' }}>{message}</p>
                <p>이미 계정이 있으신가요? <Link onClick={() => navigate('/signIn')}>로그인</Link></p>
            </Form>
        </AuthWrapper>
    );
};

const AuthWrapper = styled.div`
    display: flex;
    justify-content: center;
    align-items: center;
    background-color: #f0f0f0; 
`;

const Form = styled.form`
    background: white;
    padding: 30px;
    border-radius: 10px;
    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
    display: flex;
    flex-direction: column;
    gap: 15px;
    width: 300px;
`;

const Input = styled.input`
    padding: 10px;
    border: 1px solid #ccc;
    border-radius: 5px;
`;

const Button = styled.button`
    padding: 10px;
    background-color: #007bff;
    color: white;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    &:hover {
        background-color: #0056b3;
    }
`;

const Link = styled.span`
    color: #007bff;
    cursor: pointer;
    text-decoration: underline;
`;

export default SignUp;
